#include "communicate.h"

/**
 * This function has to be called only if the subscription of a user to a channel succeeded. It
 * sends a message to the client of the subscriber stating the subscription succeeded.
 */
void notifySubscriptionSuccess(int socket) {
    char *requestBuffer = assembleRequestContent(NWK_CLI_SUBSCRIBED, "0");

    write(socket, requestBuffer, strlen(requestBuffer) + 1);

    free(requestBuffer);
}

/**
 * This function has to be called only if the subscription cancellation of a user to a channel
 * succeeded. It sends a message to the client stating the subscription was cancelled.
 */
void notifyUnsubscriptionSuccess(int socket) {
    char *requestBuffer = assembleRequestContent(NWK_CLI_UNSUBSCRIBED, "0");

    write(socket, requestBuffer, strlen(requestBuffer) + 1);

    free(requestBuffer);
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

    free(messageBuffer);
}

/**
 * This function will subscribe the current user to the given channel (identified via its name).
 * If there is no corresponding channel, and still room in the allocated array for channels, it
 * will automatically be created.
 * 
 * The function will return its success state, to be able to destroy the thread created by the new
 * request, in case the subscription failed.
 */
subscriber *subscribeTo(int socket, char *channelName, channel **channels, int *channelsCount) {
    subscriber *sub;
    channel *chanl;

    debug("[INFO] %s: Request dispatched to the subscription service%s\n", getTime(), "", MODE);

    chanl = findOrCreateChannel(channelName, channels, channelsCount);
    if (chanl == NULL) {
        // Impossible to get or create the desired channel.
        debug("[INFO] %s: Impossible to get or create the channel '%s'\n", getTime(), channelName, MODE);
        return NULL;
    }

    sub = subscribeUser(&socket, chanl);

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
void unsubscribeFrom(subscriber *sub, char *channelName, channel **channels, int *channelsCount) {
    bool success;

    debug("[INFO] %s: Request dispatched to the unsubscription service%s\n", getTime(), "", MODE);
    success = unsubscribeUser(sub, channels, channelsCount);

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

    free(requestBuffer);
}

/**
 * Dispatches the request to the right function according to the received option code.
 * If no option code matches, this function will display an error message.
 */
void dispatchRequest(lua_State* L, subscriber *sub, int optionCode, char *messageContent, 
        char *messageBuffer, char *charTime, channel **channels, int *channelsCount) {
    char *errorMessage;

    switch (optionCode) {
        // subscribing option not allowed: here the thread is already subscribed to a channel.

        case NWK_SRV_UNSUBSCRIBE:
            unsubscribeFrom(sub, messageContent, channels, channelsCount);
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
