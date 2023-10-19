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
void browseForPollutantsAndEcologicalWords(char **messageWords, 
        char **pollutantWords, int *pollutantWordsCount, int *ecologicalWordsCount) {
    // TODO:
    // - Create the lists of pollutant and ecological words.
    // - Updating the right variable(s) on the fly.
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
short computeMessageEcoValue(char *messageContent) {
    char **messageWords;
    char **pollutantWords;
    int deltaPollutantAndEcologicalWords;
    int pollutantWordsCount = 0;
    int ecologicalWordsCount = 0;

    debug("[INFO] %s: Computing eco-value for the message from '%s'", getTime(), "???");

    // TODO:
    // - Split the message in words (by ' ').
    messageWords = malloc(0);


    pollutantWords = malloc(0); // TODO: replace 0 by the amount of words in the message
    browseForPollutantsAndEcologicalWords(
        messageWords,
        pollutantWords,
        &pollutantWordsCount,
        &ecologicalWordsCount
    );
    deltaPollutantAndEcologicalWords = ecologicalWordsCount - pollutantWordsCount;

    if ((-1) * deltaPollutantAndEcologicalWords > ECO_MAX_POLLUTANT_WORDS_TOLERATED) {
        // TODO:
        // - Replace all pollutant words (gathered within the pollutantWords array) for ecological
        //   ones.
        // - Return the opposite of the current eco-value (so that it becomes 0).
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
    // - Check that 2+1, with 2 being unsigned and 2 being signed (both shorts) equals 3.
    // - Change the eco-score value by adding the given eco-value to the stored eco-score.

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
    //   sender).
    // - Iterate through the list of subscribers for the current channel and send the message to
    //   each of them.

    debug("[INFO] %s: Message delivered to channel '%s'", getTime(), "???");
}

/**
 * This function will subscribe the current user to the given channel (identified via its name).
 * If there is no corresponding channel, and still room in the allocated array for channels, it
 * will automatically be created.
 */
void subscribeTo(char *channelName) {
    // TODO
}

/**
 * This function will unsubscribe the current user from the given channel (identified via its name).
 * If the user is the only one subscribed to that specific channel, it will be deleted.
 */
void unsubscribeFrom(char *channelName) {
    // TODO
}

/**
 * This function will send the message given to all the users connected to the same channel as the
 * current one.
 * At the same time, it will compute the eco-value of the message and update the user's eco-score
 * accordingly.
*/
void sendMessage(char *messageContent) {
    // updating the eco-score:
    short messageEcoValue = computeMessageEcoValue(messageContent);
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

    // TODO:
    // - Encapsulate the score into a response request
    // - Send the newly built request back to the client
}

/**
 * Dispatches the request to the right function according to the received option code.
 * If no option code matches, this function will display an error message.
 */
void dispatchRequest(int optionCode, char *messageContent, char *messageBuffer, char *charTime) {
    char *errorMessage;

    switch (optionCode) {
        case OPTION_SUBSCRIBE:
            subscribeTo(messageContent);
            break;

        case OPTION_UNSUBSCRIBE:
            unsubscribeFrom(messageContent);
            break;

        case OPTION_SEND_MESSAGE:
            sendMessage(messageContent);
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
void handleRequest(int socket) {
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
    dispatchRequest(optionCode, messageContent, messageBuffer, getTime());
    free(messageContent);
}

int main(int argc, char **argv) {
    int connectionSocketDescriptor;
    int transmissionSocketDescriptor;
    int currentAddressLength;
    sockaddr_in localAddress;
    sockaddr_in currentClientAddress;
    hostent *hostPointer;
    

    /* Setting up the server*/
    hostPointer = retrieveHost();
    localAddress = configureLocalAddress(hostPointer, PORT_NUMBER);
    connectionSocketDescriptor = configureConnectionSocket(localAddress);


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
        handleRequest(transmissionSocketDescriptor);
        close(transmissionSocketDescriptor);
        flushOutput();
    }
}

#pragma endregion Main algorithm functions
