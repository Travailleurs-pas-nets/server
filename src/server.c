/**
 * Server that will handle requests from many clients. It will display debug informations as well,
 * though for a production version it would be advised not to (and to write those infos in log
 * files).
 */

///////////////////////////////////////////////////////////////////////////////
//                         PROGRAMME INITIALISATION                          //
///////////////////////////////////////////////////////////////////////////////
#pragma region Programme_initialisation
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

/** Frameworks functions */
#include "../include/tpnll.h"
#include "../include/tpnn.h"

/** Generic constants */
#define LUA_STACK_TOP -1
#define LUA_MIN_SWAP_LENGTH 3
#define MAX_CHANNEL_COUNT 5
#define MAX_CHANNEL_SUBSCRIBERS_COUNT 10
#define MAX_THREAD_INACTIVITY_TIME 3000 // 5 minutes
#define PORT_NUMBER 5000
#define REQUEST_QUEUE_SIZE 5
#define MODE LL_DEBUG

/** Defining eco-score computation constants */
#define ECO_MAX 100
#define ECO_BASE 50
#define ECO_MAX_POLLUTANT_WORDS_TOLERATED 15
#define ECO_LENGTH_PENALTY_0_BOUNDARY 150 // chars
#define ECO_LENGTH_PENALTY_1_BOUNDARY 250 // chars
#define ECO_LENGTH_PENALTY_2_BOUNDARY 600 // chars


/** Defining the structures */
typedef struct sockaddr sockaddr;
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
    unsigned short ecoScore;
    /** The channel the subscriber is subscribed to. */
    channel* chanl;
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

#pragma endregion Programme_initialisation

///////////////////////////////////////////////////////////////////////////////
//                           SERVER CRUD FUNCTIONS                           //
///////////////////////////////////////////////////////////////////////////////
#pragma region Server_CRUD_functions

/**
 * This function configures the server's connection socket. It creates an IPv4 TCP socket, and
 * binds it, before to return the socket descriptor.
 */
int configureConnectionSocket(sockaddr_in localAddress) {
    int connectionSocketDescriptor;
    int operationResultCode;

    connectionSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (connectionSocketDescriptor < 0) {
        handleCriticalError("Failed to create the connection socket\n", getTime(), MODE);
    }

    operationResultCode = bind(
        connectionSocketDescriptor,
        (sockaddr *)&localAddress,
        sizeof(localAddress)
    );
    if (operationResultCode < 0) {
        handleCriticalError("Failed to bind the socket to the connection address\n", getTime(), MODE);
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
        handleCriticalError("Failed to initialise the Lua API!\n", getTime(), MODE);
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
        handleCriticalError("Failed to find 'create_dictionaries' global function!\n", getTime(), MODE);
    }

    lua_pcall(L, 0, 0, 0);
    // Now the two dictionaries in the script are in the Lua stack, full of words.
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
                handleRuntimeError("Failed to find 'swap_for_ecological_word' global function!\n", getTime(), MODE);
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
        handleRuntimeError("Channel array unexpectedly full. => Count corrected.\n", getTime(), MODE);
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
        handleRuntimeError("Failed to delete a channel. => NULL pointer given.\n", getTime(), MODE);
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
        handleRuntimeError("Channel not found within the array. => memory freed anyway.\n", getTime(), MODE);
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
 * Retrieves the eco-score for the current user in the current channel.
 */
unsigned short getEcoScore(subscriber *sub) {
    return sub->ecoScore;
}

/**
 * This function will add the given eco-value to the current eco-score.
 */
void updateEcoScore(subscriber *sub, short ecoValue) {
    sub->ecoScore += ecoValue;

    // call to getter may be removed later because we will have the value there.
    debug("[INFO] %s: New eco-score = %s\n", getTime(), intToChars(getEcoScore(sub)), MODE);
}

#pragma endregion Server_CRUD_functions

///////////////////////////////////////////////////////////////////////////////
//                              FLOW FUNCTIONS                               //
///////////////////////////////////////////////////////////////////////////////
#pragma region Flow_functions

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
        handleRuntimeError("Failed to find 'is_word_pollutant' global function!\n", getTime(), MODE);
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
        handleRuntimeError("Failed to find 'is_word_ecological' global function!\n", getTime(), MODE);
        return false;
    }

    // Pushing the parameter to the Lua API
    lua_pushstring(L, word);
    lua_pcall(L, 1, 1, 0);
    return lua_toboolean(L, LUA_STACK_TOP); // Retrieving the returned value from the Lua stack.
}

/**
 * This function will try to add a user (identified via its transfer socket descriptor) to the list
 * of subscribers of the given channel.
 * It may fail if the amount of users subscribed to the channel is equal to the maximum amount
 * allowed. In that case it will return NULL.
 */
subscriber *subscribeUser(int socket, channel *chanl) {
    subscriber *sub = malloc(sizeof(subscriber));
    bool isInserted = false;

    if (chanl->subscribersCount >= MAX_CHANNEL_SUBSCRIBERS_COUNT) {
        return NULL;
    }

    for (int i = 0; i < MAX_CHANNEL_SUBSCRIBERS_COUNT; i++) {
        if (chanl->subscribers[i] == NULL) {
            sub->transferSocket = &socket;
            sub->ecoScore = ECO_BASE;
            sub->chanl = chanl;

            chanl->subscribers[i] = sub;
            isInserted = true;
            break;
        }
    }

    if (!isInserted) {
        chanl->subscribersCount = MAX_CHANNEL_SUBSCRIBERS_COUNT;
        handleRuntimeError("Channel subscribers array unexpectedly full. => Count corrected.\n", getTime(), MODE);
        return NULL;
    }

    chanl->subscribersCount++;
    return sub;
}

/**
 * This function will try to remove a user (identified via its transfer socket descriptor) from the
 * list of subscribers of the given channel.
 * It may fail if there is no user subscribed to the channel, or if the user that wants to
 * unsubscribe isn't actually subscribed to the channel.
 */
bool unsubscribeUser(subscriber *sub) {
    bool isUnsubscribed = false;

    if (sub->chanl->subscribersCount <= 0) {
        return false;
    }

    for (int i = 0; i < MAX_CHANNEL_SUBSCRIBERS_COUNT; i++) {
        if (sub->chanl->subscribers[i]->transferSocket == sub->transferSocket) {
            // Safely deleting the subscriber
            close(*(sub->chanl->subscribers[i]->transferSocket));
            free(sub->chanl->subscribers[i]);

            sub->chanl->subscribers[i] = NULL; // Useful?
            sub->chanl->subscribersCount--;
            isUnsubscribed = true;

            // Just in case
            sub->chanl = NULL;
            break;
        }
    }

    if (sub->chanl->subscribersCount == 0) {
        deleteChannel(sub->chanl);
    }

    return isUnsubscribed;
}

/**
 * This function has to be called only if the subscription of a user to a channel succeeded. It
 * sends a message to the client of the subscriber stating the subscription succeeded.
 */
void notifySubscriptionSuccess(int socket) {
    char *requestBuffer = assembleRequestContent(NWK_CLI_SUBSCRIBED, "0");

    write(socket, requestBuffer, strlen(requestBuffer) + 1);
}

/**
 * This function has to be called only if the subscription cancellation of a user to a channel
 * succeeded. It sends a message to the client stating the subscription was cancelled.
 */
void notifyUnsubscriptionSuccess(int socket) {
    char *requestBuffer = assembleRequestContent(NWK_CLI_UNSUBSCRIBED, "0");

    write(socket, requestBuffer, strlen(requestBuffer) + 1);
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
short computeMessageEcoValue(lua_State *L, char *messageContent, unsigned short current) {
    char **messageWords;
    char **pollutantWords;
    unsigned short wordCount;
    int deltaPollutantAndEcologicalWords;
    int pollutantWordsCount = 0;
    int ecologicalWordsCount = 0;

    debug("[INFO] %s: Computing eco-value for the message from '%s'\n", getTime(), "???", MODE);

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
        return ((-1) * current);
    }

    /* We want to cap the increase at the same value as we tolerate pollutant words. */
    return (deltaPollutantAndEcologicalWords > ECO_MAX_POLLUTANT_WORDS_TOLERATED?
                ECO_MAX_POLLUTANT_WORDS_TOLERATED: deltaPollutantAndEcologicalWords
        ) - computeMessageLengthEcoPenalty(strlen(messageContent));
}

/**
 * Will iterate through the list of users subscribed to the current channel, and send the message
 * to all of them.
 */
void deliverMessage(char *messageContent, channel *chanl) {
    char *messageBuffer;

    debug("[INFO] %s: delivering the message '%s'\n", getTime(), messageContent, MODE);

    messageBuffer = assembleRequestContent(NWK_CLI_DISTRIBUTE_MESSAGE, messageContent);
    for (int i = 0; i < MAX_CHANNEL_SUBSCRIBERS_COUNT; i++) {
        if (chanl->subscribers[i] != NULL) {
            write(*chanl->subscribers[i]->transferSocket, messageBuffer, strlen(messageBuffer) + 1);
        }
    }

    debug("[INFO] %s: Message delivered to channel '%s'\n", getTime(), "???", MODE);
}

/**
 * This function will subscribe the current user to the given channel (identified via its name).
 * If there is no corresponding channel, and still room in the allocated array for channels, it
 * will automatically be created.
 * 
 * The function will return its success state, to be able to destroy the thread created by the new
 * request, in case the subscription failed.
 */
subscriber *subscribeTo(int socket, char *channelName) {
    subscriber *sub;
    channel *chanl;

    debug("[INFO] %s: Request dispatched to the subscription service%s\n", getTime(), "", MODE);

    chanl = findOrCreateChannel(channelName);
    if (chanl == NULL) {
        // Impossible to get or create the desired channel.
        debug("[INFO] %s: Impossible to get or create the channel '%s'\n", getTime(), channelName, MODE);
        return NULL;
    }

    sub = subscribeUser(socket, chanl);

    if (sub == NULL) {
        debug("[INFO] %s: Impossible to add the user to the channel '%s'\n", getTime(), channelName, MODE);
        return NULL;
    }

    debug("[INFO] %s: User successfully connected to the channel '%s'\n", getTime(), channelName, MODE);
    notifySubscriptionSuccess(socket);
    return sub;
}

/**
 * This function will unsubscribe the current user from the given channel (identified via its name).
 * If the user is the only one subscribed to that specific channel, it will be deleted.
 */
void unsubscribeFrom(subscriber *sub, char *channelName) {
    bool success;

    debug("[INFO] %s: Request dispatched to the unsubscription service%s\n", getTime(), "", MODE);
    success = unsubscribeUser(sub);

    if (!success) {
        debug("[INFO] %s: Impossible to remove the user from the channel '%s'\n", getTime(), channelName, MODE);
        return;
    }

    debug("[INFO] %s: User successfully disconnected from the channel '%s'\n", getTime(), channelName, MODE);
    notifyUnsubscriptionSuccess(*sub->transferSocket);

    // Closing the socket to end the transmission
    close(*sub->transferSocket);
}

/**
 * This function will send the message given to all the users connected to the same channel as the
 * current one.
 * At the same time, it will compute the eco-value of the message and update the user's eco-score
 * accordingly.
 */
void sendMessage(lua_State *L, subscriber *sub, char *messageContent) {
    debug("[INFO] %s: Request dispatched to the message delivering service%s\n", getTime(), "", MODE);
    // updating the eco-score:
    short messageEcoValue = computeMessageEcoValue(L, messageContent, getEcoScore(sub));
    updateEcoScore(sub, messageEcoValue);

    // delivering the message to the subscribed clients
    deliverMessage(messageContent, sub->chanl);
}

/**
 * Function that gets the eco-score for the current client, and formats it into a request before to
 * send the result back to the client.
 */
void communicateEcoScore(subscriber *sub) {
    debug("[INFO] %s: Request dispatched to the eco-score command service%s\n", getTime(), "", MODE);
    unsigned short ecoScore = getEcoScore(sub);
    char *requestBuffer = assembleRequestContent(NWK_CLI_SEND_ECO_SCORE, intToChars(ecoScore));
    
    write(*sub->transferSocket, requestBuffer, strlen(requestBuffer) + 1);
}

/**
 * Dispatches the request to the right function according to the received option code.
 * If no option code matches, this function will display an error message.
 */
void dispatchRequest(lua_State* L, subscriber *sub, int optionCode, char *messageContent, char *messageBuffer, char *charTime) {
    char *errorMessage;

    switch (optionCode) {
        // subscribing option not allowed: here the thread is already subscribed to a channel.

        case NWK_SRV_UNSUBSCRIBE:
            unsubscribeFrom(sub, messageContent);
            break;

        case NWK_SRV_SEND_MESSAGE:
            sendMessage(L, sub, messageContent);
            break;

        case NWK_SRV_GET_ECO_SCORE:
            communicateEcoScore(sub);
            break;

        default:
            /* Invalid code => displaying the error */
            errorMessage = concat(3, "Invalid option code ('", messageBuffer, "')\n");

            handleRuntimeError(errorMessage, charTime, MODE);
            free(errorMessage);
    }
}

#pragma endregion Flow_functions

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
bool handleRequest(lua_State *L, subscriber *sub) {
    char *messageBuffer;
    char *messageContent;
    int optionCode;

    /* Reading request content */
    messageBuffer = retrieveMessage(*sub->transferSocket, MODE);

    /* Parsing the operation code and message and sending the request to the right treatment func */
    messageContent = parseMessage(messageBuffer, &optionCode);
    dispatchRequest(L, sub, optionCode, messageContent, messageBuffer, getTime());
    free(messageContent);

    if (optionCode == NWK_SRV_UNSUBSCRIBE) {
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
    subscriber *sub;
    time_t startTime;
    bool disconnected = false;
    request *threadRequest = (request *)threadParam;

    debug("[INFO] %s: Request being received... %s\n", getTime(), "", MODE);
    messageBuffer = retrieveMessage(*threadRequest->transferSocket, MODE);
    messageContent = parseMessage(messageBuffer, &optionCode);

    if (optionCode != NWK_SRV_SUBSCRIBE) {
        handleRuntimeError("First request must always be a subscription!\n", getTime(), MODE);
        flushOutput(MODE);
        return NULL; // Incorrect option code, this will kill the current thread.
    }
    sub = subscribeTo(*threadRequest->transferSocket, messageContent);
    if (sub == NULL) {
        handleRuntimeError("Failed to connect to a channel. Connection closed.\n", getTime(), MODE);
        flushOutput(MODE);
        return NULL;
    }
    flushOutput(MODE);

    // Connection is all setup. Other request types may now de received from the socket.
    startTime = time(NULL);
    while (!disconnected && (time(NULL) - startTime <= MAX_THREAD_INACTIVITY_TIME)) {
        disconnected = !handleRequest(threadRequest->L, sub);
        flushOutput(MODE);
    }

    // In case we got out of the loop because of inactivity, we disconnect the user manually
    if (!disconnected) {
        unsubscribeFrom(sub, messageContent);
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
    localAddress = configureLocalAddress(hostPointer, PORT_NUMBER, NWK_SERVER);
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
    debug("[INFO] %s: Server started listening on port %s\n\n", getTime(), intToChars(PORT_NUMBER), MODE);

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
            handleRuntimeError("Connection with client failed", getTime(), MODE);
            continue;
        }

        threadRequest.L = L;
        threadRequest.transferSocket = &transmissionSocketDescriptor;
        pthread_create(&transferThread, NULL, onRequestReceived, (void *)&threadRequest);
    }
}

#pragma endregion Main algorithm functions
