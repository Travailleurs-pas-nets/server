#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/crud/crud_channel.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_deleteChannelSubscribers(const MunitParameter params[], void *data) {
    channel **chanls = malloc(sizeof(channel *) * MAX_CHANNEL_COUNT);
    int channel_count = 0;

    // Silencing the compiler
    (void) data;
    (void) params;

    channel *chanl = createChannel("0", chanls, &channel_count);

    // mem liberation
    deleteChannelSubscribers(chanls[0]);
    free(chanl);
    free(chanls);

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
