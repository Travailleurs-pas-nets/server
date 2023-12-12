#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/crud/crud_channel.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_findOrCreateChannel(const MunitParameter params[], void *data) {
    channel **chanls = malloc(sizeof(channel *) * MAX_CHANNEL_COUNT);
    int channel_count = 0;

    // Silencing the compiler
    (void) data;
    (void) params;

    // 3 cases:
    // - channel is in the list;
    // - channel is not in the list and can be added;
    // - channel is not in the list and cannot be added.

    for (int i = 0; i < MAX_CHANNEL_COUNT-1; i++) {
        createChannel(intToChars(i), chanls, &channel_count);
    }

    channel *chanl_obtained = findOrCreateChannel("0", chanls, &channel_count);
    munit_assert_not_null(chanl_obtained);
    munit_assert_int32(channel_count, ==, MAX_CHANNEL_COUNT-1);

    chanl_obtained = findOrCreateChannel("4", chanls, &channel_count);
    munit_assert_not_null(chanl_obtained);
    munit_assert_int32(channel_count, ==, MAX_CHANNEL_COUNT);

    chanl_obtained = findOrCreateChannel("5", chanls, &channel_count);
    munit_assert_null(chanl_obtained);
    munit_assert_int32(channel_count, ==, MAX_CHANNEL_COUNT);


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
