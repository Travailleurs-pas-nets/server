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
#include <pthread.h>        /* For threads */
#define LUA_STACK_TOP -1
#define LUA_MIN_SWAP_LENGTH 3
#define MAX_HOST_NAME_LENGTH 256
#define MAX_MESSAGE_LENGTH 1024
#define MAX_CHANNEL_COUNT 5
#define MAX_CHANNEL_SUBSCRIBERS_COUNT 10
#define MAX_THREAD_INACTIVITY_TIME 3000 // 5 minutes
#define PORT_NUMBER 5000
#define REQUEST_QUEUE_SIZE 5
#define DEBUG 1

/** Defining option codes */
#define OPTION_CODE_LENGTH 24
#define OPTION_SUBSCRIBE 0
#define OPTION_UNSUBSCRIBE 1
#define OPTION_SEND_MESSAGE 2
#define OPTION_GET_ECO_SCORE 3

/** Defining client option codes */
#define CLI_SUBSCRIBED 0
#define CLI_UNSUBSCRIBED 1
#define CLI_DISTRIBUTE_MESSAGE 2
#define CLI_SEND_ECO_SCORE 3
#define CLI_DISTRIBUTE_REMINDER 4

/** Defining eco-score computation constants */
#define ECO_MIN 0
#define ECO_MAX 100
#define ECO_BASE 50
#define ECO_MAX_POLLUTANT_WORDS_TOLERATED 15
#define ECO_LENGTH_PENALTY_0_BOUNDARY 150 // chars
#define ECO_LENGTH_PENALTY_1_BOUNDARY 250 // chars
#define ECO_LENGTH_PENALTY_2_BOUNDARY 600 // chars


/** Defining the structures */
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;
typedef struct subscriber subscriber;
typedef struct channel channel;
typedef struct request request;

struct subscriber {
    /** 
     * The transfer socket of the user. This is useful because even though the user thread may be
     * terminated, the socket will not necessarily be closed.
     */
    int *transferSocket;
    /** The user's eco-score. */
    int ecoScore;
};

struct channel {
    /** The name of the channel. */
    char *name;
    /** The array of subscribers, (containing their transfer socket identifiers). */
    subscriber **subscribers;
    /** The count of subscribers to the channel. */
    unsigned short subscribersCount;
};

struct request {
    /** The global lua thread, giving access to dictionary functions. */
    lua_State *L;
    /** The transfer socket thanks to which the server will be able to discuss with the client. */
    int *transferSocket;
};

/** Creating global variables */
channel *channels[MAX_CHANNEL_COUNT] = {0};
int channelsCount = 0;

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


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
/**
 * Function that splits a string into an array of strings, using the given delimiter.
 * 
 * The pragmas are because we use values pointed to, which will trigger a warning at compile time,
 * saying a computed value is not used, while it actually is.
 */
char **split(char *stringToSplit, char delimiter, unsigned short *wordCount) {
    char *token;
    char **tokens;
    char delimiterAsString[1] = { delimiter };
    unsigned short delimiterCount = 0;
    unsigned short localWordCount = 0;
    char *stringDuplicate = strdup(stringToSplit); // Copying the string to ensure it stays unchanged.

    // First, we iterate through the string to find the number of times the delimiter is present.
    for (int i = 0; stringDuplicate[i] != '\0'; i++) {
        if (stringDuplicate[i] == delimiter) {
            delimiterCount++;
        }
    }

    // Then we create an array of a capacity of 1 more than the delimiter count (because there is
    // one more element than there are delimiters).
    tokens = malloc(sizeof(char *) * (delimiterCount + 2));

    // And finally we fill our array with strings.
    token = strtok(stringDuplicate, delimiterAsString);
    while (token) {
        tokens[localWordCount] = token;
        localWordCount++;
        token = strtok(NULL, delimiterAsString);
    }
    
    *wordCount = localWordCount;
    tokens[localWordCount + 1] = 0;
    return tokens;
}
#pragma GCC diagnostic pop

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
 * Function that creates a request's option code from its unsigned short representation.
 */
char *assembleOptionCode(unsigned short optionCode) {
    char *codeValue;
    char *optionCodeString = malloc(sizeof(char) * (OPTION_CODE_LENGTH + 1));

    codeValue = intToChars(optionCode);
    optionCodeString[0] = '0';
    for (int i = 1; i <= strlen(codeValue); i++) {
        optionCodeString[i] = codeValue[i - 1];
    }
    for (int i = strlen(codeValue) + 1; i < OPTION_CODE_LENGTH; i++) {
        optionCodeString[i] = ' ';
    }

    optionCodeString[OPTION_CODE_LENGTH] = '\0';
    return optionCodeString;
}

/**
 * Function that creates a request content from its code and message.
 */
char *assembleRequestContent(unsigned short optionCode, char *content) {
    char *optionCodeString = assembleOptionCode(optionCode);
    return concat(2, optionCodeString, content);
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

/**
 * Function reading a request content from a socket.
 * It will display an error message if a received message is empty, and return an empty buffer in
 * that case.
*/
char *retrieveMessage(int transferSocket) {
    int messageLength;
    char *messageBuffer = malloc(MAX_MESSAGE_LENGTH * sizeof(char));

    messageLength = read(transferSocket, messageBuffer, sizeof(messageBuffer));
    if (messageLength <= 0) {
        handleRuntimeError("Empty message received\n", getTime());
    }
    debug("[INFO] %s: Message received => '%s'\n", getTime(), messageBuffer);

    return messageBuffer;
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
    char *machineName = malloc(MAX_HOST_NAME_LENGTH + 1);
    gethostname(machineName, MAX_HOST_NAME_LENGTH);
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
        char *errorMessage = concat(3, "Server not found ('", hostName, "')\n");
        handleCriticalError(errorMessage, getTime());
    }

    return host;
}

/**
 * Will configure the local address to the given host and port.
 */
sockaddr_in configureLocalAddress(hostent *host, unsigned short port) {
    sockaddr_in localAddress;

    bcopy((char *)host->h_addr_list[0], (char *)&localAddress.sin_addr, host->h_length);
    localAddress.sin_family = host->h_addrtype;
    localAddress.sin_addr.s_addr = INADDR_ANY;

    localAddress.sin_port = htons(port);

    return localAddress;
}

/**
 * This function configures the server's connection socket. It creates an IPv4 TCP socket, and
 * binds it, before to return the socket descriptor.
 */
int configureConnectionSocket(sockaddr_in localAddress) {
    int connectionSocketDescriptor;
    int operationResultCode;

    connectionSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (connectionSocketDescriptor < 0) {
        handleCriticalError("Failed to create the connection socket\n", getTime());
    }

    operationResultCode = bind(
        connectionSocketDescriptor,
        (sockaddr *)&localAddress,
        sizeof(localAddress)
    );
    if (operationResultCode < 0) {
        handleCriticalError("Failed to bind the socket to the connection address\n", getTime());
    }

    return connectionSocketDescriptor;
}

/**
 * Function to initialise the Lua API.
*/
lua_State *initialiseLua() {
    int executionState;
    lua_State *L = luaL_newstate(); // called L by convention
    luaL_openlibs(L);
    
    executionState = luaL_dofile(L, "./bin/scripts/dictionaries.lua");
    if (executionState != LUA_OK) {
        handleCriticalError("Failed to initialise the Lua API!\n", getTime());
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
        handleCriticalError("Failed to find 'create_dictionaries' global function!\n", getTime());
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
        handleRuntimeError("Failed to find 'is_word_pollutant' global function!\n", getTime());
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
        handleRuntimeError("Failed to find 'is_word_ecological' global function!\n", getTime());
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
        const char *word;

        if (strlen(pollutantWords[i]) < LUA_MIN_SWAP_LENGTH) {
            // Word shorter than the shortest word in the ecological dictionary
            word = createQuestionMarkString(strlen(pollutantWords[i]));
        } else {
            lua_getglobal(L, "swap_for_ecological_word"); // pushes the function to the top of the stack.

            if (!lua_isfunction(L, LUA_STACK_TOP)) {
                handleRuntimeError("Failed to find 'swap_for_ecological_word' global function!\n", getTime());
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

/**
 * This function allocates memory for the subscribers array contained in the channel structure, and
 * initialises each space to a NULL value, which will later be meaning that the place is free.
 */
void initialiseChannelSubscribers(channel *chanl) {
    chanl->subscribers = malloc(MAX_CHANNEL_SUBSCRIBERS_COUNT * sizeof(subscriber *));

    for(int i = 0; i < MAX_CHANNEL_SUBSCRIBERS_COUNT; i++) {
        chanl->subscribers[i] = NULL;
    }
}

/**
 * Function that searches through the list of channels already on the server for a channel with the
 * given name.
 * If no matching channel is found, a NULL pointer is returned.
 */
channel *findChannelByName(const char *channelName) {
    channel *chanl = NULL;

    for (int i = 0; i < MAX_CHANNEL_COUNT; i++) {
        if (strcmp(channels[i]->name, channelName) == 0) {
            chanl = channels[i];
        }
    }

    return chanl;
}

/**
 * Creates a channel with the given channel name.
 * This function will fail if the amount of channel on the server is already equal to the maximum
 * amount defined by a constant.
 * In that case, the returned value will be a NULL pointer.
 */
channel *createChannel(char *channelName) {
    channel *chanl;
    bool isInserted = false;

    if (channelsCount >= MAX_CHANNEL_COUNT) {
        return NULL;
    }

    // Creating the channel.
    chanl = malloc(sizeof(channel));
    chanl->name = channelName;
    chanl->subscribersCount = 0;
    initialiseChannelSubscribers(chanl);

    // Now that the instance is created, we try to add it to the array.
    for (int i = 0; i < MAX_CHANNEL_COUNT; i++) {
        if (channels[i] == 0) {
            channels[i] = chanl;
            isInserted = true;
            break;
        }
    }

    if (!isInserted) {
        channelsCount = MAX_CHANNEL_COUNT;
        handleRuntimeError("Channel array unexpectedly full. => Count corrected.\n", getTime());
        free(chanl->subscribers);
        free(chanl);
        return NULL;
    }

    channelsCount++;
    return chanl;
}

/**
 * Function that safely deletes the subscribers of a channel.
 */
void deleteChannelSubscribers(channel *chanl) {
    for (int i = 0; i < MAX_CHANNEL_SUBSCRIBERS_COUNT; i++) {
        if (chanl->subscribers[i] != NULL) {
            // Closing every sockets, just in case.
            close(*(chanl->subscribers[i]->transferSocket));
            free(chanl->subscribers[i]);
        }
    }
}

/**
 * Deletes the given channel.
 * This function will fail if the given pointer is `NULL`. It may also display an error message if
 * the channel given to be deleted was not in the channels list. In that case, the channel will
 * still be deleted anyway, so please do not use it afterwards.
*/
void deleteChannel(channel *chanl) {
    bool isDeleted = false;

    if (chanl == NULL) {
        handleRuntimeError("Failed to delete a channel. => NULL pointer given.\n", getTime());
        return;
    }

    // Removing the channel from the list
    for (int i = 0; i < MAX_CHANNEL_COUNT; i++) {
        if (channels[i] == chanl) { // Effectively looking for a reference
            channels[i] = NULL; // Removing the pointer to the given channel from the channel pointers list
            isDeleted = true;
            break;
        }
    }

    if (!isDeleted) {
        handleRuntimeError("Channel not found within the array. => memory freed anyway.\n", getTime());
    }

    // Freeing the memory
    deleteChannelSubscribers(chanl);
    free(chanl->subscribers);
    free(chanl);
}

/**
 * Function that will look for a channel with the given name, or create a new channel if it isn't
 * found.
 * If it isn't found, and it isn't possible to create a new one, this function will return a NULL
 * pointer.
 */
channel *findOrCreateChannel(char *channelName) {
    channel *chanl = findChannelByName(channelName);
    if (chanl == NULL) {
        chanl = createChannel(channelName);
    }

    return chanl;
}

/**
 * This function will try to add a user (identified via its transfer socket descriptor) to the list
 * of subscribers of the given channel.
 * It may fail if the amount of users subscribed to the channel is equal to the maximum amount
 * allowed. In that case it will return false.
 */
bool subscribeUser(int socket, channel *chanl) {
    subscriber sub;
    bool isInserted = false;

    if (chanl->subscribersCount >= MAX_CHANNEL_SUBSCRIBERS_COUNT) {
        return false;
    }

    for (int i = 0; i < MAX_CHANNEL_SUBSCRIBERS_COUNT; i++) {
        if (chanl->subscribers[i] == NULL) {
            sub.transferSocket = &socket;
            sub.ecoScore = ECO_BASE;

            chanl->subscribers[i] = &sub;
            isInserted = true;
            break;
        }
    }

    if (!isInserted) {
        chanl->subscribersCount = MAX_CHANNEL_SUBSCRIBERS_COUNT;
        handleRuntimeError("Channel subscribers array unexpectedly full. => Count corrected.\n", getTime());
        return false;
    }

    chanl->subscribersCount++;
    return true;
}

/**
 * This function will try to remove a user (identified via its transfer socket descriptor) from the
 * list of subscribers of the given channel.
 * It may fail if there is no user subscribed to the channel, or if the user that wants to
 * unsubscribe isn't actually subscribed to the channel.
*/
bool unsubscribeUser(int socket, channel *chanl) {
    bool isUnsubscribed = false;

    if (chanl->subscribersCount <= 0) {
        return false;
    }

    for (int i = 0; i < MAX_CHANNEL_SUBSCRIBERS_COUNT; i++) {
        if (chanl->subscribers[i]->transferSocket == &socket) {
            // Safely deleting the subscriber
            close(*(chanl->subscribers[i]->transferSocket));
            free(chanl->subscribers[i]);

            chanl->subscribers[i] = NULL; // Useful?
            chanl->subscribersCount--;
            isUnsubscribed = true;
            break;
        }
    }

    if (chanl->subscribersCount == 0) {
        deleteChannel(chanl);
    }

    return isUnsubscribed;
}

/**
 * This function has to be called only if the subscription of a user to a channel succeeded. It
 * sends a message to the client of the subscriber stating the subscription succeeded.
 */
void notifySubscriptionSuccess(int socket) {
    char *requestBuffer = assembleRequestContent(CLI_SUBSCRIBED, "0");

    write(socket, requestBuffer, strlen(requestBuffer) + 1);
}

/**
 * This function has to be called only if the subscription cancellation of a user to a channel
 * succeeded. It sends a message to the client stating the subscription was cancelled.
 */
void notifyUnsubscriptionSuccess(int socket) {
    char *requestBuffer = assembleRequestContent(CLI_UNSUBSCRIBED, "0");

    write(socket, requestBuffer, strlen(requestBuffer) + 1);
}

#pragma endregion Server framework functions

///////////////////////////////////////////////////////////////////////////////
//                              FLOW FUNCTIONS                               //
///////////////////////////////////////////////////////////////////////////////
#pragma region Flow functions

/**
 * This function will separate the option code from the message content.
 */
char *parseMessage(char *messageBuffer, int *optionCode) {
    char *messageContent = strdup(messageBuffer + OPTION_CODE_LENGTH);

    messageBuffer[OPTION_CODE_LENGTH] = '\0';
    sscanf(messageBuffer, "%d", optionCode);

    return messageContent;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
/**
 * Function that will iterate through the words within the message from the client, trying to
 * identify pollutant or ecological words, and editing the content of the three last variables
 * accordingly.
*/
void browseForPollutantsAndEcologicalWords(lua_State *L, char **messageWords, 
        char **pollutantWords, int *pollutantWordsCount, int *ecologicalWordsCount) {
    int localPollutantWordsCount = 0;
    int localEcologicalWordsCount = 0;

    while (*messageWords != 0) {
        if (isWordPollutant(L, *messageWords)) {
            pollutantWords[localPollutantWordsCount] = *messageWords;
            localPollutantWordsCount++;
        } else if (isWordEcological(L, *messageWords)) {
            localEcologicalWordsCount++;
        }

        ++messageWords;
    }

    *pollutantWordsCount = localPollutantWordsCount;
    *ecologicalWordsCount = localEcologicalWordsCount;
}
#pragma GCC diagnostic pop

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

    debug("[INFO] %s: Computing eco-value for the message from '%s'\n", getTime(), "???");

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
 * Retrieves the eco-score for the current user in the current channel.
 */
unsigned short getEcoScore() {
    // TODO:
    // - Store the eco-score somewhere it is possible to retrieve during a request.
    return 0;
}

/**
 * This function will add the given eco-value to the current eco-score.
 */
void updateEcoScore(short ecoValue) {
    // TODO:
    // - Change the eco-score value by adding the given eco-value to the stored eco-score => thread gesture

    // call to getter may be removed later because we will have the value there.
    // +, not sure the default conversion from unsigned short to int is valid.
    debug("[INFO] %s: New eco-score = %s\n", getTime(), intToChars(getEcoScore()));
}

/**
 * Will iterate through the list of users subscribed to the current channel, and send the message
 * to all of them.
 */
void deliverMessage(char *messageContent) {
    char *messageBuffer;

    debug("[INFO] %s: delivering the message '%s'\n", getTime(), messageContent);

    messageBuffer = assembleRequestContent(CLI_DISTRIBUTE_MESSAGE, messageContent);

    // TODO:
    // - Iterate through the list of subscribers for the current channel and send the message to
    //   each of them => thread gesture.

    debug("[INFO] %s: Message delivered to channel '%s'\n", getTime(), "???");
}

/**
 * This function will subscribe the current user to the given channel (identified via its name).
 * If there is no corresponding channel, and still room in the allocated array for channels, it
 * will automatically be created.
 * 
 * The function will return its success state, to be able to destroy the thread created by the new
 * request, in case the subscription failed.
 */
bool subscribeTo(int socket, char *channelName) {
    bool success;
    channel *chanl;

    debug("[INFO] %s: Request dispatched to the subscription service%s\n", getTime(), "");

    chanl = findOrCreateChannel(channelName);
    if (chanl == NULL) {
        // Impossible to get or create the desired channel.
        debug("[INFO] %s: Impossible to get or create the channel '%s'\n", getTime(), channelName);
        return false;
    }

    success = subscribeUser(socket, chanl);

    if (!success) {
        debug("[INFO] %s: Impossible to add the user to the channel '%s'\n", getTime(), channelName);
        return false;
    }

    // TODO:
    // - Add a pointer to the channel the user connected itself to, to its transfer thread => thread gesture
    debug("[INFO] %s: User successfully connected to the channel '%s'\n", getTime(), channelName);
    notifySubscriptionSuccess(socket);
    return true;
}

/**
 * This function will unsubscribe the current user from the given channel (identified via its name).
 * If the user is the only one subscribed to that specific channel, it will be deleted.
 */
void unsubscribeFrom(int socket, char *channelName) {
    bool success;
    channel *chanl;

    debug("[INFO] %s: Request dispatched to the unsubscription service%s\n", getTime(), "");
    
    chanl = findChannelByName(channelName);
    if (chanl == NULL) {
        handleRuntimeError("No matching channel", getTime());
        return;
    }

    success = unsubscribeUser(socket, chanl);

    if (!success) {
        debug("[INFO] %s: Impossible to remove the user from the channel '%s'\n", getTime(), channelName);
        return;
    }

    // TODO:
    // - Set the pointer to the channel the user disconnected itself from, in its transfer thread
    //   to NULL => thread gesture
    debug("[INFO] %s: User successfully disconnected from the channel '%s'\n", getTime(), channelName);
    notifyUnsubscriptionSuccess(socket);

    // Closing the socket to end the transmission
    close(socket);
}

/**
 * This function will send the message given to all the users connected to the same channel as the
 * current one.
 * At the same time, it will compute the eco-value of the message and update the user's eco-score
 * accordingly.
*/
void sendMessage(lua_State *L, int socket, char *messageContent) {
    debug("[INFO] %s: Request dispatched to the message delivering service%s\n", getTime(), "");
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
void communicateEcoScore(int socket) {
    debug("[INFO] %s: Request dispatched to the eco-score command service%s\n", getTime(), "");
    unsigned short ecoScore = getEcoScore();
    char *requestBuffer = assembleRequestContent(CLI_SEND_ECO_SCORE, intToChars(ecoScore));
    
    write(socket, requestBuffer, strlen(requestBuffer) + 1);
}

/**
 * Dispatches the request to the right function according to the received option code.
 * If no option code matches, this function will display an error message.
 */
void dispatchRequest(lua_State* L, int socket, int optionCode, char *messageContent, char *messageBuffer, char *charTime) {
    char *errorMessage;

    switch (optionCode) {
        // subscribing option not allowed: here the thread is already subscribed to a channel.

        case OPTION_UNSUBSCRIBE:
            unsubscribeFrom(socket, messageContent);
            break;

        case OPTION_SEND_MESSAGE:
            sendMessage(L, socket, messageContent);
            break;

        case OPTION_GET_ECO_SCORE:
            communicateEcoScore(socket);
            break;

        default:
            /* Invalid code => displaying the error */
            errorMessage = concat(3, "Invalid option code ('", messageBuffer, "')\n");

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
 * 
 * At the end of the request, this function will escalate whether the request was a subscription
 * cancellation to its caller.
 */
bool handleRequest(lua_State *L, int socket) {
    char *messageBuffer;
    char *messageContent;
    int optionCode;

    /* Reading request content */
    messageBuffer = retrieveMessage(socket);

    /* Parsing the operation code and message and sending the request to the right treatment func */
    messageContent = parseMessage(messageBuffer, &optionCode);
    dispatchRequest(L, socket, optionCode, messageContent, messageBuffer, getTime());
    free(messageContent);

    if (optionCode == OPTION_UNSUBSCRIBE) {
        return false;
    }
    return true;
}

void *onRequestReceived(void *threadParam) {
    // For the first request of the thread, we make sure it is a channel subscription. Any other
    // request will be thrown away, and the thread will be destroyed immediately after.
    char *messageBuffer;
    char *messageContent;
    int optionCode;
    time_t startTime;
    bool disconnected = false;
    request *threadRequest = (request *)threadParam;

    debug("[INFO] %s: Request being received... %s\n", getTime(), "");
    messageBuffer = retrieveMessage(*threadRequest->transferSocket);
    messageContent = parseMessage(messageBuffer, &optionCode);

    if (optionCode != OPTION_SUBSCRIBE) {
        handleRuntimeError("First request must always be a subscription!\n", getTime());
        flushOutput();
        return NULL; // Incorrect option code, this will kill the current thread.
    }
    if (!subscribeTo(*threadRequest->transferSocket, messageContent)) {
        handleRuntimeError("Failed to connect to a channel. Connection closed.\n", getTime());
        flushOutput();
        return NULL;
    }
    flushOutput();

    // Connection is all setup. Other request types may now de received from the socket.
    startTime = time(NULL);
    while (!disconnected && (time(NULL) - startTime <= MAX_THREAD_INACTIVITY_TIME)) {
        disconnected = !handleRequest(threadRequest->L, *threadRequest->transferSocket);
        flushOutput();
    }

    // In case we got out of the loop because of inactivity, we disconnect the user manually
    if (!disconnected) {
        unsubscribeFrom(*threadRequest->transferSocket, messageContent);
    }

    return NULL;
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

    // TODO:
    // - Take the dictionaries out of lua, in order to transfer memory ownership to the c programme
    // - Threads will have to own their own lua thread, giving them access to the read-only
    //   dictionaries, and this way, the stack interferences that may occur will vanish away in
    //   oblivion. 🥳
    //   => Lua scripts may have to be broken down to separate files.


    /* Handling requests */
    listen(connectionSocketDescriptor, REQUEST_QUEUE_SIZE);
    debug("[INFO] %s: Server started listening on port %s\n\n", getTime(), intToChars(PORT_NUMBER));

    for (;;) { // Listening for incoming requests
        pthread_t transferThread;
        request threadRequest;

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

        threadRequest.L = L;
        threadRequest.transferSocket = &transmissionSocketDescriptor;
        pthread_create(&transferThread, NULL, onRequestReceived, (void *)&threadRequest);
    }
}

#pragma endregion Main algorithm functions
