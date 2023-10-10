/**
 * Server that will handle requests from many clients. It will display debug informations as well,
 * though for a production version it would be advised not to (and to write those infos in log
 * files).
 */
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
#define MAX_NAME_LENGTH 256
#define MAX_MESSAGE_LENGTH 1024
#define PORT_NUMBER 5001
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

char *getTime() {
    time_t rawTime = time(NULL);
    char *charTime = ctime(&rawTime);
    charTime[strlen(charTime) - 1] = '\0'; /* Removing the line break at the end. */

    return charTime;
}

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

void flushOutput() {
    printf("\n");
}

/**
 * Function that handles errors.
 * If the debug mode is enabled, these will be displayed in the terminal, otherwise they will be
 * written to a log file.
 */
void handleError(char *message, char *charTime) {
    char *errorMessage = malloc(strlen(message) + strlen(charTime) + 13); // 13 being for additional characters & \0.
    strcpy(errorMessage, "[ERROR] ");
    strcat(errorMessage, charTime);
    strcat(errorMessage, ": ");
    strcat(errorMessage, message);

    if (DEBUG == 1) {
        perror(errorMessage);
    } else {
        // TODO (writing in a log file)
    }

    free(errorMessage);
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
            errorMessage = malloc(strlen(messageBuffer) + 25);
            strcpy(errorMessage, "Invalid option code ('");
            strcat(errorMessage, messageBuffer);
            strcat(errorMessage, "')");

            handleError(errorMessage, charTime);
            free(errorMessage);
    }
}

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
        handleError("Empty message received", charTime);
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
    int operationResultCode;
    sockaddr_in localAddress;
    sockaddr_in currentClientAddress;
    hostent *hostPointer;
    servent *servicePointer;
    char machineName[MAX_NAME_LENGTH + 1];
    char *errorMessage;
    char *charTime = getTime();

    gethostname(machineName, MAX_NAME_LENGTH);

    hostPointer = gethostbyname(machineName);
    if (hostPointer == NULL) {
        errorMessage = malloc(strlen(machineName) + 23);
        strcpy(errorMessage, "Server not found  ('");
        strcat(errorMessage, machineName);
        strcat(errorMessage, "')");

        handleError(errorMessage, charTime);
        free(errorMessage);
        exit(1);
    }

    bcopy((char *)hostPointer->h_addr, (char *)&localAddress.sin_addr, hostPointer->h_length);
    localAddress.sin_family = hostPointer->h_addrtype;
    localAddress.sin_addr.s_addr = INADDR_ANY;

    localAddress.sin_port = htons(PORT_NUMBER);

    connectionSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (connectionSocketDescriptor < 0) {
        exit(1);
    }

    operationResultCode = bind(
        connectionSocketDescriptor,
        (sockaddr *)&localAddress,
        sizeof(localAddress)
    );
    if (operationResultCode < 0) {
        handleError("Failed to bind the socket to the connection address", charTime);
        exit(1);
    }

    listen(connectionSocketDescriptor, REQUEST_QUEUE_SIZE);
    debug("[INFO] %s: Server started listening on port %s\n", charTime, intToChars(PORT_NUMBER));

    for (;;) {
        currentAddressLength = sizeof(currentClientAddress);

        transmissionSocketDescriptor = accept(
            connectionSocketDescriptor,
            (sockaddr *)&currentClientAddress,
            &currentAddressLength
        );
        if (transmissionSocketDescriptor < 0) {
            handleError("Connection with client failed", getTime());
            continue;
        }

        debug("[INFO] %s: Request being received... %s\n", getTime(), "");

        handleRequest(transmissionSocketDescriptor);
        close(transmissionSocketDescriptor);
        flushOutput();
    }
}
