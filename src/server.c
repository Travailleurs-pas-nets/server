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
#define OPTION_MAX_CODE 1   /* Needs to be equal to the greatest option code! */
#define OPTION_SEND_MESSAGE 0
#define OPTION_SUBSCRIBE 1

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
 * Dispatches the request to the right function according to the received option code.
 * If no option code matches, this function will display an error message.
 */
void dispatchRequest(int optionCode, char *messageContent, char *messageBuffer, char *charTime) {
    char *errorMessage;

    switch (optionCode) {
        case OPTION_SEND_MESSAGE:
            break;

        case OPTION_SUBSCRIBE:
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
    char messageBuffer[MAX_MESSAGE_LENGTH];
    int messageLength;
    char *messageContent;
    int optionCode;
    char *charTime = getTime();

    /* Reading request content */
    messageLength = read(socket, messageBuffer, sizeof(messageBuffer));
    if (messageLength <= 0) {
        handleRuntimeError("Empty message received", charTime);
    }
    debug("[INFO] %s: Message received => '%s'\n", charTime, messageBuffer);

    /* Parsing the operation code and message and sending the request to the right treatment func */
    parseMessage(messageBuffer, messageContent, &optionCode);
    dispatchRequest(optionCode, messageContent, messageBuffer, charTime);
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
