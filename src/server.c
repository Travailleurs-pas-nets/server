/**
 * Server that will handle requests from many clients. It will display debug informations as well,
 * though for a production version it would be advised not to (and to write those infos in log
 * files).
 */

///////////////////////////////////////////////////////////////////////////////
//                         PROGRAMME INITIALISATION                          //
///////////////////////////////////////////////////////////////////////////////
#pragma region Programme_initialisation
#include "std_libs.h"
#include "lua_libs.h"       /** Lua headers */
#include "homemade_libs.h"  /** Homemade functions & constants headers */

/** Include components: */
#include "crud/crud_socket.h"
#include "crud/crud_lua.h"
#include "crud/crud_channel.h"
#include "crud/crud_subscriber.h"

#include "business/pollution_computation.h"
#include "business/communicate.h"


/** Creating global variables */
channel *channels[MAX_CHANNEL_COUNT] = {0};
int channelsCount = 0;

#pragma endregion Programme_initialisation

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
int handleRequest(lua_State *L, subscriber *sub) {
    char *messageBuffer;
    char *messageContent;
    int optionCode;

    /* Reading request content */
    messageBuffer = retrieveMessage(*sub->transferSocket, MODE);
    if (strlen(messageBuffer) < 24) {
        // Request is rejected because there aren't even enough characters for the operation code.
        // Therefore we return and leave this method early to make the inactivity verification possible.
        //
        // We do not return false though because being in this case could simply just mean that we are
        // waiting for the user's input.
        return TREATMENT_NOT_EXECUTED;
    }

    /* Parsing the operation code and message and sending the request to the right treatment func */
    messageContent = parseMessage(messageBuffer, &optionCode, MODE);
    dispatchRequest(L, sub, optionCode, messageContent, messageBuffer, getTime(), channels, &channelsCount);

    free(messageBuffer);
    free(messageContent);

    if (optionCode == NWK_SRV_UNSUBSCRIBE) {
        return TREATMENT_DISCONNECTED;
    }
    return TREATMENT_EXECUTED;
}

void *onRequestReceived(void *threadParam) {
    // For the first request of the thread, we make sure it is a channel subscription. Any other
    // request will be thrown away, and the thread will be destroyed immediately after.
    char *messageBuffer;
    char *messageContent;
    int optionCode;
    subscriber *sub;
    time_t startTime;
    int requestTreatment = TREATMENT_NOT_EXECUTED;
    request *threadRequest = (request *)threadParam;

    debug("[INFO] %s: Request being received... %s\n", getTime(), "", MODE);
    messageBuffer = retrieveMessage(threadRequest->transferSocket, MODE);
    debug("[INFO] %s: Starting to parse the message... %s\n", getTime(), "", MODE);
    messageContent = parseMessage(messageBuffer, &optionCode, MODE);
    debug("[INFO] %s: Trying to free the message buffer... %s\n", getTime(), "", MODE);
    free(messageBuffer);

    debug("[INFO] %s: Checking that the request is a subscription... %s\n", getTime(), "", MODE);
    if (optionCode != NWK_SRV_SUBSCRIBE) {
        handleRuntimeError("First request must always be a subscription!\n", getTime(), MODE);
        flushOutput(MODE);
        return NULL; // Incorrect option code, this will kill the current thread.
    }
    debug("[INFO] %s: Trying to subscribe to a channel... %s\n", getTime(), "", MODE);
    sub = subscribeTo(threadRequest->transferSocket, messageContent, channels, &channelsCount);
    debug("[INFO] %s: Checking subscription state... %s\n", getTime(), "", MODE);
    if (sub == NULL) {
        handleRuntimeError("Failed to connect to a channel. Connection closed.\n", getTime(), MODE);
        flushOutput(MODE);
        return NULL;
    }
    flushOutput(MODE);

    // Connection is all setup. Other request types may now be received from the socket.
    startTime = time(NULL);
    while (requestTreatment != TREATMENT_DISCONNECTED && (time(NULL) - startTime <= MAX_THREAD_INACTIVITY_TIME)) {
        requestTreatment = handleRequest(threadRequest->L, sub);
        
        if (requestTreatment == TREATMENT_EXECUTED) {
            startTime = time(NULL);
            // User did something, so they are not inactive. Timer before inaction deletion reset.
        }
        flushOutput(MODE);
    }

    // In case we got out of the loop because of inactivity, we disconnect the user manually
    if (requestTreatment != TREATMENT_DISCONNECTED) {
        unsubscribeFrom(sub, messageContent, channels, &channelsCount);
    }

    free(messageContent);

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
    hostPointer = retrieveHost(MODE);
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
        } else if (transmissionSocketDescriptor == 0) {
            continue;
        }

        threadRequest.L = L;
        threadRequest.transferSocket = transmissionSocketDescriptor;
        pthread_create(&transferThread, NULL, onRequestReceived, (void *)&threadRequest);
        transmissionSocketDescriptor = 0;
    }
}

#pragma endregion Main algorithm functions
