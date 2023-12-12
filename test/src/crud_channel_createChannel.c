#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/crud/crud_channel.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_createChannel(const MunitParameter params[], void *data) {
    channel **chanls = malloc(sizeof(channel *) * MAX_CHANNEL_COUNT);
    int channel_count = 0;

    // Silencing the compiler
    (void) data;
    (void) params;

    for (int i = 0; i < MAX_CHANNEL_COUNT+1; i++) {
        channel *chanl = createChannel(intToChars(i), chanls, &channel_count);

        if (i < MAX_CHANNEL_COUNT) {
            munit_assert_not_null(chanl);
            munit_assert_int32(channel_count, ==, i+1);
        } else {
            munit_assert_null(chanl);
            munit_assert_int32(channel_count, ==, i);
        }
    }

    // mem liberation
    for (int i = 0; i < MAX_CHANNEL_COUNT; i++) {
        channel *chanl = chanls[i];
        free(chanl->subscribers);
        free(chanl);
    }
    free(chanls);

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
