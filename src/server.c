/**
 * Server that will handle requests from many clients. It will display debug informations as well,
 * though for a production version it would be advised not to (and to write those infos in log
 * files).
 */

///////////////////////////////////////////////////////////////////////////////
//                         PROGRAMME INITIALISATION                          //
///////////////////////////////////////////////////////////////////////////////
#pragma region Programme initialisation
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>    /* For the sockets */
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h> 		    /* For hostent, servent */
#include <string.h> 		/* For bcopy, ... */  
#include <time.h>           /* To get the date and time */
#include <math.h>           /* To get pow() and floor() functions */
#include <stdbool.h>        /* For booleans...... */
#include <stdarg.h>         /* For variadic functions */
#include <lua.h>            /* Lua lib, allowing to incorporate Lua code to C programs */
#include <lauxlib.h>
#include <lualib.h>
#define LUA_STACK_TOP -1
#define LUA_MIN_SWAP_LENGTH 3
#define MAX_NAME_LENGTH 256
#define MAX_MESSAGE_LENGTH 1024
#define PORT_NUMBER 5000
#define REQUEST_QUEUE_SIZE 5
#define DEBUG 1

/** Defining option codes */
#define OPTION_CODE_LENGTH 24
#define OPTION_SUBSCRIBE 0
#define OPTION_UNSUBSCRIBE 1
#define OPTION_SEND_MESSAGE 2
#define OPTION_GET_ECO_SCORE 3

/** Defining eco-score computation constants */
#define ECO_MAX_POLLUTANT_WORDS_TOLERATED 15
#define ECO_LENGTH_PENALTY_0_BOUNDARY 150 // chars
#define ECO_LENGTH_PENALTY_1_BOUNDARY 250 // chars
#define ECO_LENGTH_PENALTY_2_BOUNDARY 600 // chars


/** Defining the structures */
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

#pragma endregion Programme initialisation

///////////////////////////////////////////////////////////////////////////////
//                            FRAMEWORK FUNCTIONS                            //
///////////////////////////////////////////////////////////////////////////////
#pragma region Framework functions

/**
 * Function that returns the current time (when calling the method) as a char array.
 */
char *getTime() {
    time_t rawTime = time(NULL);
    char *charTime = ctime(&rawTime);
    charTime[strlen(charTime) - 1] = '\0'; /* Removing the line break at the end. */

    return charTime;
}

/**
 * Function that converts an integer in a char array.
*/
char *intToChars(int in) {
    char *result;
    int nextIndex = 0;
    bool startedWriting = false;

    if (in >= 0) {
        int size = 10; /* Max length of a 32 bits int. */
        result = malloc(size);
    } else {
        int size = 11; /* +1 for the minus sign. */
        result = malloc(size);
        result[nextIndex] = '-';
        nextIndex++;
    }

    for (int power = 9; power >= 0; power--) {
        int value = (int) floor(in / pow(10, power));

        /* second condition because we don't want to stop writing the string once we started,
           otherwise we will lose powers of ten in the resulting array of characters. */
        if (value >= 1 || startedWriting) {
            result[nextIndex] = value + 48;
            nextIndex++;

            in -= value * (int)pow(10, power);
            startedWriting = true;
        }
    }

    result[nextIndex] = '\0';
    return result;
}

/**
 * Function that concatenates the given amount parameters.
 */
char *concat(unsigned short argCount, ...) {
    va_list argList;
    char *concatenated;
    int size = 1;
    
    va_start(argList, argCount);
    for (int i = 0; i < argCount; i++) {
        size += strlen(va_arg(argList, char *));
    }
    va_end(argList);
    
    concatenated = malloc(size + 1);
    *concatenated = '\0';

    va_start(argList, argCount);
    for (int i = 0; i < argCount; i++) {
        strcat(concatenated, va_arg(argList, char *));
    }
    va_end(argList);

    return concatenated;
}

/**
 * Function that splits a string into an array of strings, using the given delimiter.
 */
char **split(char *stringToSplit, char delimiter, unsigned short *wordCount) {
    char *token;
    char **tokens;
    unsigned short delimiterCount = 0;
    *wordCount = 0;

    // First, we iterate through the string to find the number of times the delimiter is present.
    for (int i = 0; stringToSplit[i] != '\0'; i++) {
        if (stringToSplit[i] == delimiter) {
            delimiterCount++;
        }
    }

    // Then we create an array of a capacity of 1 more than the delimiter count (because there is
    // one more element than there are delimiters).
    tokens = malloc(sizeof(char *) * (delimiterCount + 1));

    // And finally we fill our array with strings.
    token = strtok(stringToSplit, delimiter);
    while (token) {
        tokens[*wordCount] = token;
        *wordCount++;
        token = strtok(NULL, delimiter);
    }
    
    return tokens;
}

/**
 * Function that creates a string of the given length, filled with question marks.
 */
char *createQuestionMarkString(int length) {
    char *word = malloc(length);

    for (int i = 0; i < length - 1; i++) {
        word[i] = '?';
    }

    word[length - 1] = '\0';
    return word;
}

/**
 * Function that flushes the console output in debug mode.
 */
void flushOutput() {
    if (DEBUG == 1) {
        printf("\n");
    }
}

/**
 * Function that handles errors.
 * If the debug mode is enabled, these will be displayed in the terminal, otherwise they will be
 * written to a log file.
 */
void handleError(char *errorTag, char *message, char *charTime) {
    char *errorMessage = concat(5, errorTag, " ", charTime, ": ", message);

    if (DEBUG == 1) {
        perror(errorMessage);
    } else {
        // TODO (writing in a log file)
    }

    //free(errorMessage);
}

/**
 * Function that handles runtime errors.
 * These errors takes the `[ERROR]` tag.
 */
void handleRuntimeError(char *message, char *charTime) {
    handleError("[ERROR]", message, charTime);
}

/**
 * Function that handles critical errors.
 * These errors takes the `[CRITICAL]` tag and causes the program to stop.
*/
void handleCriticalError(char *message, char *charTime) {
    handleError("[CRITICAL]", message, charTime);
    exit(1);
}

/**
 * Function that handles standard debug display.
 * If the debug mode is enabled, it will display the informations in the terminal, otherwise it
 * won't display anything.
 */
void debug(const char *messageTemplate, char *charTime, char *content) {
    if (DEBUG == 1) {
        printf(messageTemplate, charTime, content);
    }
}

#pragma endregion Framework functions

///////////////////////////////////////////////////////////////////////////////
//                        SERVER FRAMEWORK FUNCTIONS                         //
///////////////////////////////////////////////////////////////////////////////
#pragma region Server framework functions

/**
 * Wrapper for the unix `gethostname()` function. This allows normalised calls to standard methods.
*/
char *getMachineName() {
    char *machineName = malloc(MAX_NAME_LENGTH + 1);
    gethostname(machineName, MAX_NAME_LENGTH);
    return machineName;
}

/**
 * Function that will create and return a pointer towards the hostent.
 * It will throw a critical error if the retrieved host is null before return.
 */
hostent *retrieveHost() {
    hostent *host;
    char *hostName;
    
    hostName = getMachineName();
    host = gethostbyname(hostName);

    if (host == NULL) {
        char *errorMessage = concat(3, "Server not found ('", hostName, "')");
        handleCriticalError(errorMessage, getTime());
    }

    return host;
}

/**
 * Will configure the local address to the given host and port.
 */
sockaddr_in configureLocalAddress(hostent *host, unsigned short port) {
    sockaddr_in localAddress;

    bcopy((char *)host->h_addr, (char *)&localAddress.sin_addr, host->h_length);
    localAddress.sin_family = host->h_addrtype;
    localAddress.sin_addr.s_addr = INADDR_ANY;

    localAddress.sin_port = htons(port);

    return localAddress;
}

int configureConnectionSocket(sockaddr_in localAddress) {
    int connectionSocketDescriptor;
    int operationResultCode;

    connectionSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (connectionSocketDescriptor < 0) {
        handleCriticalError("Failed to create the connection socket", getTime());
    }

    operationResultCode = bind(
        connectionSocketDescriptor,
        (sockaddr *)&localAddress,
        sizeof(localAddress)
    );
    if (operationResultCode < 0) {
        handleCriticalError("Failed to bind the socket to the connection address", getTime());
    }

    return connectionSocketDescriptor;
}

/**
 * Function to initialise the Lua API.
*/
lua_State *initialiseLua() {
    int executionState;
    lua_State *L = lua_open(); // called L by convention
    luaL_openlibs(L);
    
    executionState = luaL_dofile(L, "./dictionaries.lua");
    if (executionState != LUA_OK) {
        handleCriticalError("Failed to initialise the Lua API!", getTime());
    }

    return L;
}

/**
 * This function will build the ecological and pollutant words dictionaries (not talking about the
 * structural pattern here, but the first meaning of a dictionary). It will use the C Lua API,
 * and the dictionaries will be stored in the Lua stack.
 */
void buildDictionaries(lua_State *L) {
    lua_getglobal(L, "create_dictionaries"); // pushes the function to the top of the stack.

    if (!lua_isfunction(L, LUA_STACK_TOP)) {
        handleCriticalError("Failed to find 'create_dictionaries' global function!", getTime());
    }

    lua_pcall(L, 0, 0, 0);
    // Now the two dictionaries in the script are in the Lua stack, full of words.
}

/**
 * Tells whether the given word is considered pollutant or not.
 * - `true` (1) if it is;
 * - `false` (0) otherwise.
 * 
 * Note: the time complexity of this function in the worst case scenario is O(n), where n is the
 * number of letters in the word to check, not the number of words in the dictionary!
 */
bool isWordPollutant(lua_State *L, char *word) {
    lua_getglobal(L, "is_word_pollutant"); // pushes the function to the top of the stack.

    if (!lua_isfunction(L, LUA_STACK_TOP)) {
        handleRuntimeError("Failed to find 'is_word_pollutant' global function!", get_time());
        return false;
    }

    // Pushing the parameter to the Lua API
    lua_pushstring(L, word);
    lua_pcall(L, 1, 1, 0);
    return lua_toboolean(L, LUA_STACK_TOP); // Retrieving the returned value from the Lua stack
}

/**
 * Tells whether the given word is considered ecological or not.
 * - `true` (1) if it is;
 * - `false` (0) otherwise.
 * 
 * Note: the time complexity of this function in the worst case scenario is O(n), where n is the
 * number of letters in the word to check, not the number of words in the dictionary!
 */
bool isWordEcological(lua_State *L, char *word) {
    lua_getglobal(L, "is_word_ecological"); // pushes the function to the top of the stack.

    if (!lua_isfunction(L, LUA_STACK_TOP)) {
        handleRuntimeError("Failed to find 'is_word_ecological' global function!", getTime());
        return false;
    }

    // Pushing the parameter to the Lua API
    lua_pushstring(L, word);
    lua_pcall(L, 1, 1, 0);
    return lua_toboolean(L, LUA_STACK_TOP); // Retrieving the returned value from the Lua stack.
}

/**
 * Will replace every pollutant word by ecological words of the same length randomly.
 */
void replacePollutantWords(lua_State *L, char **pollutantWords, int count) {
    for (int i = 0; i < count; i++) {
        char *word;

        if (strlen(pollutantWords[i]) < LUA_MIN_SWAP_LENGTH) {
            // Word shorter than the shortest word in the ecological dictionary
            word = createQuestionMarkString(strlen(pollutantWords[i]));
        } else {
            lua_getglobal(L, "swap_for_ecological_word"); // pushes the function to the top of the stack.

            if (!lua_isfunction(L, LUA_STACK_TOP)) {
                handleRuntimeError("Failed to find 'swap_for_ecological_word' global function!", getTime());
                return;
            }

            // Pushing the parameter to the Lua API
            lua_pushnumber(L, strlen(pollutantWords[i]));
            lua_pcall(L, 1, 1, 0);
            word = lua_tostring(L, LUA_STACK_TOP);
        }

        // Replacing the pollutant word's letters one by one.
        for (int j = 0; j < strlen(word); j++) {
            pollutantWords[i][j] = word[j];
        }
    }
}

#pragma endregion Server framework functions

///////////////////////////////////////////////////////////////////////////////
//                              FLOW FUNCTIONS                               //
///////////////////////////////////////////////////////////////////////////////
#pragma region Flow functions

/**
 * This function will separate the option code from the message content.
 */
void parseMessage(char *messageBuffer, char *messageContent, int *optionCode) {
    messageContent = strdup(messageBuffer + OPTION_CODE_LENGTH + 1);
    messageBuffer[OPTION_CODE_LENGTH + 1] = '\0';
    sscanf(messageBuffer, "%d", optionCode);
}

/**
 * Function that will iterate through the words within the message from the client, trying to
 * identify pollutant or ecological words, and editing the content of the three last variables
 * accordingly.
*/
void browseForPollutantsAndEcologicalWords(lua_State *L, char **messageWords, 
        char **pollutantWords, int *pollutantWordsCount, int *ecologicalWordsCount) {
    *pollutantWordsCount = 0;
    *ecologicalWordsCount = 0;

    while (*messageWords != 0) {
        if (isWordPollutant(L, *messageWords)) {
            pollutantWords[*pollutantWordsCount] = *messageWords;
            *pollutantWordsCount++;
        } else if (isWordEcological(L, *messageWords)) {
            *ecologicalWordsCount++;
        }

        ++messageWords;
    }
}

/**
 * Function that computes the eco-penalty depending on the message length
 */
short computeMessageLengthEcoPenalty(int charCount) {
    if (charCount <= ECO_LENGTH_PENALTY_0_BOUNDARY) {
        return 0;
    } else if (charCount <= ECO_LENGTH_PENALTY_1_BOUNDARY) {
        return 1;
    } else if (charCount <= ECO_LENGTH_PENALTY_2_BOUNDARY) {
        return 2;
    } else {
        return 3;
    }
}

/**
 * This function will compute the eco-value of a message. This value will then have to be added to
 * the current eco-value. It may be negative.
 */
short computeMessageEcoValue(lua_State *L, char *messageContent) {
    char **messageWords;
    char **pollutantWords;
    unsigned short wordCount;
    int deltaPollutantAndEcologicalWords;
    int pollutantWordsCount = 0;
    int ecologicalWordsCount = 0;

    debug("[INFO] %s: Computing eco-value for the message from '%s'", getTime(), "???");

    messageWords = split(messageContent, ' ', &wordCount);


    pollutantWords = malloc(sizeof(char *) * wordCount);
    browseForPollutantsAndEcologicalWords(
        L,
        messageWords,
        pollutantWords,
        &pollutantWordsCount,
        &ecologicalWordsCount
    );
    deltaPollutantAndEcologicalWords = ecologicalWordsCount - pollutantWordsCount;

    if ((-1) * deltaPollutantAndEcologicalWords > ECO_MAX_POLLUTANT_WORDS_TOLERATED) {
        replacePollutantWords(L, pollutantWords, pollutantWordsCount);
        // TODO:
        // - Return the opposite of the current eco-score (so that it becomes 0) => thread gesture
    }

    /* We want to cap the increase at the same value as we tolerate pollutant words. */
    return (deltaPollutantAndEcologicalWords > ECO_MAX_POLLUTANT_WORDS_TOLERATED?
                ECO_MAX_POLLUTANT_WORDS_TOLERATED: deltaPollutantAndEcologicalWords
        ) - computeMessageLengthEcoPenalty(strlen(messageContent));
}

/**
 * This function will add the given eco-value to the current eco-score.
 */
void updateEcoScore(short ecoValue) {
    // TODO:
    // - Change the eco-score value by adding the given eco-value to the stored eco-score => thread gesture

    // call to getter may be removed later because we will have the value there.
    // +, not sure the default conversion from unsigned short to int is valid.
    debug("[INFO] %s: New eco-score = %s", getTime(), intToChars(getEcoScore()));
}

/**
 * Retrieves the eco-score for the current user in the current channel.
 */
unsigned short getEcoScore() {
    // TODO:
    // - Store the eco-score somewhere it is possible to retrieve during a request.
}

/**
 * Will iterate through the list of users subscribed to the current channel, and send the message
 * to all of them.
*/
void deliverMessage(char *messageContent) {
    debug("[INFO] %s: delivering the message '%s'", getTime(), messageContent);

    // TODO:
    // - Prepare a request for the client, containing the response (and potentially the name of the
    //   sender) => needs documentation from client on how to send requests to them.
    // - Iterate through the list of subscribers for the current channel and send the message to
    //   each of them => thread gesture.

    debug("[INFO] %s: Message delivered to channel '%s'", getTime(), "???");
}

/**
 * This function will subscribe the current user to the given channel (identified via its name).
 * If there is no corresponding channel, and still room in the allocated array for channels, it
 * will automatically be created.
 */
void subscribeTo(char *channelName) {
    // TODO => thread gesture.
}

/**
 * This function will unsubscribe the current user from the given channel (identified via its name).
 * If the user is the only one subscribed to that specific channel, it will be deleted.
 */
void unsubscribeFrom(char *channelName) {
    // TODO => thread gesture.
}

/**
 * This function will send the message given to all the users connected to the same channel as the
 * current one.
 * At the same time, it will compute the eco-value of the message and update the user's eco-score
 * accordingly.
*/
void sendMessage(lua_State *L, char *messageContent) {
    // updating the eco-score:
    short messageEcoValue = computeMessageEcoValue(L, messageContent);
    updateEcoScore(messageEcoValue);

    // delivering the message to the subscribed clients
    deliverMessage(messageContent);
}

/**
 * Function that gets the eco-score for the current client, and formats it into a request before to
 * send the result back to the client.
 */
void communicateEcoScore() {
    unsigned short ecoScore = getEcoScore();

    // TODO: => client requests documentation.
    // - Encapsulate the score into a response request.
    // - Send the newly built request back to the client.
}

/**
 * Dispatches the request to the right function according to the received option code.
 * If no option code matches, this function will display an error message.
 */
void dispatchRequest(lua_State* L, int optionCode, char *messageContent, char *messageBuffer, char *charTime) {
    char *errorMessage;

    switch (optionCode) {
        case OPTION_SUBSCRIBE:
            subscribeTo(messageContent);
            break;

        case OPTION_UNSUBSCRIBE:
            unsubscribeFrom(messageContent);
            break;

        case OPTION_SEND_MESSAGE:
            sendMessage(L, messageContent);
            break;

        case OPTION_GET_ECO_SCORE:
            communicateEcoScore();
            break;

        default:
            /* Invalid code => displaying the error */
            errorMessage = concat(3, "Invalid option code ('", messageBuffer, "')");

            handleRuntimeError(errorMessage, charTime);
            free(errorMessage);
    }
}

#pragma endregion Flow functions

///////////////////////////////////////////////////////////////////////////////
//                         MAIN ALGORITHM FUNCTIONS                          //
///////////////////////////////////////////////////////////////////////////////
#pragma region Main algorithm functions

/**
 * This function is the one that will handle every request.
 * As there are two kind of requests possible, this will check what type of request is made by the
 * calling client and then dispatch the execution to the right treatment function.
 * 
 * The two possible options are the following (according to the observer pattern):
 * - subscribing or unsubscribing to a discussion;
 * - sending a message in a discussion.
 * 
 * To detect what option is requested, the function will read the 24 first characters of the buffer.
 * These should contain the option code (`01`) for subscription and (`00`) for sending a message.
 */
void handleRequest(lua_State *L, int socket) {
    char messageBuffer[MAX_MESSAGE_LENGTH] = {0};
    int messageLength;
    char *messageContent;
    int optionCode;

    /* Reading request content */
    messageLength = read(socket, messageBuffer, sizeof(messageBuffer));
    if (messageLength <= 0) {
        handleRuntimeError("Empty message received", getTime());
    }
    debug("[INFO] %s: Message received => '%s'\n", getTime(), messageBuffer);

    /* Parsing the operation code and message and sending the request to the right treatment func */
    parseMessage(messageBuffer, messageContent, &optionCode);
    dispatchRequest(L, optionCode, messageContent, messageBuffer, getTime());
    free(messageContent);
}

int main(int argc, char **argv) {
    int connectionSocketDescriptor;
    int transmissionSocketDescriptor;
    int currentAddressLength;
    sockaddr_in localAddress;
    sockaddr_in currentClientAddress;
    hostent *hostPointer;
    lua_State *L; // called L by convention
    

    /* Setting up the server*/
    hostPointer = retrieveHost();
    localAddress = configureLocalAddress(hostPointer, PORT_NUMBER);
    connectionSocketDescriptor = configureConnectionSocket(localAddress);
    L = initialiseLua();
    buildDictionaries(L);


    /* Handling requests */
    listen(connectionSocketDescriptor, REQUEST_QUEUE_SIZE);
    debug("[INFO] %s: Server started listening on port %s\n", getTime(), intToChars(PORT_NUMBER));

    for (;;) {
        currentAddressLength = sizeof(currentClientAddress);

        transmissionSocketDescriptor = accept(
            connectionSocketDescriptor,
            (sockaddr *)&currentClientAddress,
            (unsigned int *restrict)&currentAddressLength
        );
        if (transmissionSocketDescriptor < 0) {
            handleRuntimeError("Connection with client failed", getTime());
            continue;
        }

        debug("[INFO] %s: Request being received... %s\n", getTime(), "");
        handleRequest(L, transmissionSocketDescriptor);
        close(transmissionSocketDescriptor);
        flushOutput();
    }
}

#pragma endregion Main algorithm functions
