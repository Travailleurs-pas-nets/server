#pragma once

/** File to include all homemade libraries & to define homemade constants */

#include "lua_libs.h"

/** Frameworks functions */
#include "../include/tpnll.h"
#include "../include/tpnn.h"

/** Generic constants */
#define MAX_CHANNEL_COUNT 5
#define MAX_CHANNEL_SUBSCRIBERS_COUNT 10
#define MAX_THREAD_INACTIVITY_TIME 3000 // 5 minutes
#define PORT_NUMBER 5000
#define REQUEST_QUEUE_SIZE 5
#define MODE LL_PROD

/** Defining eco-score computation constants */
#define ECO_MAX 100
#define ECO_BASE 50
#define ECO_MAX_POLLUTANT_WORDS_TOLERATED 15
#define ECO_LENGTH_PENALTY_0_BOUNDARY 150 // chars
#define ECO_LENGTH_PENALTY_1_BOUNDARY 250 // chars
#define ECO_LENGTH_PENALTY_2_BOUNDARY 600 // chars

/** Defining structures */
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
