#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/crud/crud_subscriber.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_unsubscribeUser(const MunitParameter params[], void *data) {
    // Silencing the compiler
    (void) data;
    (void) params;

    channel **chanls = malloc(sizeof(channel *) * MAX_CHANNEL_COUNT);
    int channels_count = 0;
    channel *chanl = createChannel("test", chanls, &channels_count);

    for (int i = 0; i < MAX_CHANNEL_SUBSCRIBERS_COUNT; i++) {
        int *socket = malloc(sizeof(int));
        int *pointer = malloc(sizeof(int));
        *pointer = i;
        memcpy(&socket, &pointer, sizeof(int *));
        subscribeUser(socket, chanl);
    }

    for (int i = MAX_CHANNEL_SUBSCRIBERS_COUNT-1; i >= 0; i--) {
        subscriber *sub = chanl->subscribers[i];
        bool success = unsubscribeUser(sub, chanls, &channels_count);

        if (i > 0) {
            munit_assert_true(success);
            munit_assert_uint16(chanl->subscribersCount, ==, i);
        } else {
            munit_assert_true(success);
            munit_assert_uint16(chanl->subscribersCount, ==, i);
            munit_assert_int32(channels_count, ==, 0);
        }

        free(sub);
    }

    
    free(chanls);

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
