#pragma once

#include "../homemade_libs.h"

/**
 * This function allocates memory for the subscribers array contained in the channel structure, and
 * initialises each space to a NULL value, which will later be meaning that the place is free.
 * 
 * ⚠️ WARNING: This function contains a hidden `malloc`, therefore, when you are done with the
 * value, you should free its memory.
 */
extern void initialiseChannelSubscribers(channel *chanl);

/**
 * Function that searches through the list of channels already on the server for a channel with the
 * given name.
 * If no matching channel is found, a NULL pointer is returned.
 */
extern channel *findChannelByName(const char *channelName, channel **channels);

/**
 * Creates a channel with the given channel name.
 * This function will fail if the amount of channel on the server is already equal to the maximum
 * amount defined by a constant.
 * In that case, the returned value will be a NULL pointer.
 * 
 * ⚠️ WARNING: This function contains a hidden `malloc`, therefore, when you are done with the
 * value, you should free its memory.
 */
extern channel *createChannel(char *channelName, channel **channels, int *channelsCount);

/**
 * Function that safely deletes the subscribers of a channel.
 */
extern void deleteChannelSubscribers(channel *chanl);

/**
 * Deletes the given channel.
 * This function will fail if the given pointer is `NULL`. It may also display an error message if
 * the channel given to be deleted was not in the channels list. In that case, the channel will
 * still be deleted anyway, so please do not use it afterwards.
 */
extern void deleteChannel(channel *chanl, channel **channels, int *channelsCount);

/**
 * Function that will look for a channel with the given name, or create a new channel if it isn't
 * found.
 * If it isn't found, and it isn't possible to create a new one, this function will return a NULL
 * pointer.
 */
extern channel *findOrCreateChannel(char *channelName, channel **channels, int *channelsCount);
