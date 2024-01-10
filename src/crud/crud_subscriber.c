#include "crud_subscriber.h"

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
    if (ecoValue*-1 > sub->ecoScore) {
        sub->ecoScore = 0;

    } else if (ecoValue + sub->ecoScore > ECO_MAX) {
        sub->ecoScore = ECO_MAX;

    } else {
        sub->ecoScore += ecoValue;
    }

    debug("[INFO] %s: New eco-score = %s\n", getTime(), intToChars(getEcoScore(sub)), MODE);
}

/**
 * This function will try to add a user (identified via its transfer socket descriptor) to the list
 * of subscribers of the given channel.
 * It may fail if the amount of users subscribed to the channel is equal to the maximum amount
 * allowed. In that case it will return NULL.
 * 
 * ⚠️ WARNING: This function contains a hidden `malloc`, therefore, when you are done with the
 * value, you should free its memory.
 */
subscriber *subscribeUser(int *socket, channel *chanl) {
    subscriber *sub = malloc(sizeof(subscriber));
    bool isInserted = false;

    if (chanl->subscribersCount >= MAX_CHANNEL_SUBSCRIBERS_COUNT) {
        return NULL;
    }

    for (int i = 0; i < MAX_CHANNEL_SUBSCRIBERS_COUNT; i++) {
        if (chanl->subscribers[i] == NULL) {
            sub->transferSocket = socket;
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
bool unsubscribeUser(subscriber *sub, channel **channels, int *channelsCount) {
    bool isUnsubscribed = false;

    if (sub->chanl->subscribersCount <= 0) {
        return false;
    }

    for (int i = 0; i < MAX_CHANNEL_SUBSCRIBERS_COUNT; i++) {
        if (sub->chanl->subscribers[i] != NULL && sub->chanl->subscribers[i]->transferSocket == sub->transferSocket) {

            // Breaking the link to the current user in the concerned channel
            sub->chanl->subscribers[i] = memcpy(sub->chanl->subscribers[i], sub, sizeof(subscriber)); // changing the pointer
            sub->chanl->subscribers[i] = NULL;
            sub->chanl->subscribersCount--;
            isUnsubscribed = true;

            // ⚠️ Warning: subscriber must be neither freed, nor closed! We need to send a notification
            // to the client before doing that!

            break;
        }
    }

    if (sub->chanl->subscribersCount == 0) {
        deleteChannel(sub->chanl, channels, channelsCount);
    }

    sub->chanl = NULL;
    return isUnsubscribed;
}
