#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/crud/crud_subscriber.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_subscribeUser(const MunitParameter params[], void *data) {
    // Silencing the compiler
    (void) data;
    (void) params;

    channel *chanl = malloc(sizeof(channel));
    chanl->name = "test";
    chanl->subscribersCount = 0;
    initialiseChannelSubscribers(chanl);

    for (int i = 0; i <= MAX_CHANNEL_SUBSCRIBERS_COUNT; i++) {
        int socket;
        memcpy(&socket, &i, sizeof(int));
        subscriber *sub = subscribeUser(&socket, chanl);

        if (i < MAX_CHANNEL_SUBSCRIBERS_COUNT) {
            munit_assert_not_null(sub);
            munit_assert_uint16(chanl->subscribersCount, ==, i+1);
        } else {
            munit_assert_null(sub);
            munit_assert_uint16(chanl->subscribersCount, ==, i);
        }
    }

    deleteChannelSubscribers(chanl);
    free(chanl);

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
