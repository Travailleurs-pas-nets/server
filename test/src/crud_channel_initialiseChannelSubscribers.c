#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/crud/crud_channel.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_initialiseChannelSubscribers(const MunitParameter params[], void *data) {
    // Silencing the compiler
    (void) data;
    (void) params;

    // Not much to test here...
    channel *chanl = malloc(sizeof(channel));

    initialiseChannelSubscribers(chanl);

    // One assertion, but more by principle than anything else. Basically, if we get there, it means
    // the function succeeded.
    munit_assert_not_null(chanl->subscribers);
    munit_assert_int32(chanl->subscribersCount, ==, 0);

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
