#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/crud/crud_channel.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_deleteChannel(const MunitParameter params[], void *data) {
    channel **chanls = malloc(sizeof(channel *) * MAX_CHANNEL_COUNT);
    int channel_count = 0;

    // Silencing the compiler
    (void) data;
    (void) params;

    for (int i = 0; i < MAX_CHANNEL_COUNT; i++) {
        createChannel(intToChars(i), chanls, &channel_count);
    }

    // mem liberation
    for (int i = MAX_CHANNEL_COUNT-1; i >= 0; i--) {
        channel *chanl = chanls[i];
        deleteChannel(chanl, chanls, &channel_count);

        munit_assert_int32(channel_count, ==, i);
    }
    free(chanls);

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
