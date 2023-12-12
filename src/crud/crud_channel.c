#include "crud_channel.h"

/**
 * This function allocates memory for the subscribers array contained in the channel structure, and
 * initialises each space to a NULL value, which will later be meaning that the place is free.
 * 
 * ⚠️ WARNING: This function contains a hidden `malloc`, therefore, when you are done with the
 * value, you should free its memory.
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
channel *findChannelByName(const char *channelName, channel **channels) {
    channel *chanl = NULL;

    for (int i = 0; i < MAX_CHANNEL_COUNT; i++) {
        if (channels[i] != NULL && strcmp(channels[i]->name, channelName) == 0) {
            chanl = channels[i];
        }
    }

    return chanl;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
/**
 * Creates a channel with the given channel name.
 * This function will fail if the amount of channel on the server is already equal to the maximum
 * amount defined by a constant.
 * In that case, the returned value will be a NULL pointer.
 * 
 * ⚠️ WARNING: This function contains a hidden `malloc`, therefore, when you are done with the
 * value, you should free its memory.
 */
channel *createChannel(char *channelName, channel **channels, int *channelsCount) {
    channel *chanl;
    bool isInserted = false;
    int localChannelsCount = *channelsCount;

    if (localChannelsCount >= MAX_CHANNEL_COUNT) {
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
        *channelsCount = MAX_CHANNEL_COUNT;
        handleRuntimeError("Channel array unexpectedly full. => Count corrected.\n", getTime(), MODE);
        free(chanl->subscribers);
        free(chanl);
        return NULL;
    }

    *channelsCount = ++localChannelsCount;
    return chanl;
}
#pragma GCC diagnostic pop

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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
/**
 * Deletes the given channel.
 * This function will fail if the given pointer is `NULL`. It may also display an error message if
 * the channel given to be deleted was not in the channels list. In that case, the channel will
 * still be deleted anyway, so please do not use it afterwards.
 */
void deleteChannel(channel *chanl, channel **channels, int *channelsCount) {
    bool isDeleted = false;
    int localChannelsCount = *channelsCount;

    if (chanl == NULL) {
        handleRuntimeError("Failed to delete a channel. => NULL pointer given.\n", getTime(), MODE);
        return;
    }

    // Removing the channel from the list
    for (int i = 0; i < MAX_CHANNEL_COUNT; i++) {
        if (channels[i] == chanl) { // Effectively looking for a reference
            channels[i] = NULL; // Removing the pointer to the given channel from the channel pointers list
            localChannelsCount--;
            isDeleted = true;
            break;
        }
    }

    if (!isDeleted) {
        handleRuntimeError("Channel not found within the array. => memory freed anyway.\n", getTime(), MODE);
    }

    *channelsCount = localChannelsCount;

    // Freeing the memory
    deleteChannelSubscribers(chanl);
    free(chanl->subscribers);
    free(chanl);
}
#pragma GCC diagnostic pop

/**
 * Function that will look for a channel with the given name, or create a new channel if it isn't
 * found.
 * If it isn't found, and it isn't possible to create a new one, this function will return a NULL
 * pointer.
 */
channel *findOrCreateChannel(char *channelName, channel **channels, int *channelsCount) {
    channel *chanl = findChannelByName(channelName, channels);
    if (chanl == NULL) {
        chanl = createChannel(channelName, channels, channelsCount);
    }

    return chanl;
}
