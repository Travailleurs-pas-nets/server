#pragma once

#include "../homemade_libs.h"
#include "crud_channel.h"

/**
 * Retrieves the eco-score for the current user in the current channel.
 */
extern unsigned short getEcoScore(subscriber *sub);

/**
 * This function will add the given eco-value to the current eco-score.
 */
extern void updateEcoScore(subscriber *sub, short ecoValue);

/**
 * This function will try to add a user (identified via its transfer socket descriptor) to the list
 * of subscribers of the given channel.
 * It may fail if the amount of users subscribed to the channel is equal to the maximum amount
 * allowed. In that case it will return NULL.
 * 
 * ⚠️ WARNING: This function contains a hidden `malloc`, therefore, when you are done with the
 * value, you should free its memory.
 */
extern subscriber *subscribeUser(int socket, channel *chanl);

/**
 * This function will try to remove a user (identified via its transfer socket descriptor) from the
 * list of subscribers of the given channel.
 * It may fail if there is no user subscribed to the channel, or if the user that wants to
 * unsubscribe isn't actually subscribed to the channel.
 */
extern bool unsubscribeUser(subscriber *sub, channel **channels, int *channelsCount);
