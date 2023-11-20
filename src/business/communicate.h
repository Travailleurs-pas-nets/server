#pragma once

#include "../lua_libs.h"
#include "../homemade_libs.h"

#include "../crud/crud_channel.h"
#include "../crud/crud_subscriber.h"

#include "pollution_computation.h"

/**
 * This function has to be called only if the subscription of a user to a channel succeeded. It
 * sends a message to the client of the subscriber stating the subscription succeeded.
 */
extern void notifySubscriptionSuccess(int socket);

/**
 * This function has to be called only if the subscription cancellation of a user to a channel
 * succeeded. It sends a message to the client stating the subscription was cancelled.
 */
extern void notifyUnsubscriptionSuccess(int socket);

/**
 * Will iterate through the list of users subscribed to the current channel, and send the message
 * to all of them.
 */
extern void deliverMessage(char *messageContent, channel *chanl);

/**
 * This function will subscribe the current user to the given channel (identified via its name).
 * If there is no corresponding channel, and still room in the allocated array for channels, it
 * will automatically be created.
 * 
 * The function will return its success state, to be able to destroy the thread created by the new
 * request, in case the subscription failed.
 */
extern subscriber *subscribeTo(int socket, char *channelName, channel **channels, int *channelsCount);

/**
 * This function will unsubscribe the current user from the given channel (identified via its name).
 * If the user is the only one subscribed to that specific channel, it will be deleted.
 */
extern void unsubscribeFrom(subscriber *sub, char *channelName, channel **channels, int *channelsCount);

/**
 * This function will send the message given to all the users connected to the same channel as the
 * current one.
 * At the same time, it will compute the eco-value of the message and update the user's eco-score
 * accordingly.
 */
extern void sendMessage(lua_State *L, subscriber *sub, char *messageContent);

/**
 * Function that gets the eco-score for the current client, and formats it into a request before to
 * send the result back to the client.
 */
extern void communicateEcoScore(subscriber *sub);

/**
 * Dispatches the request to the right function according to the received option code.
 * If no option code matches, this function will display an error message.
 */
extern void dispatchRequest(lua_State* L, subscriber *sub, int optionCode, char *messageContent,
        char *messageBuffer, char *charTime, channel **channels, int *channelsCount);
