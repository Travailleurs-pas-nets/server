#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/crud/crud_channel.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_findChannelByName(const MunitParameter params[], void *data) {
    channel *chanl_obtained;
    char *chanl_name;

    channel *test  = malloc(sizeof(channel));
    channel *test1 = malloc(sizeof(channel));
    channel *test2 = malloc(sizeof(channel));

    test->name = "test";
    test1->name = "test1";
    test2->name = "test2";

    // Silencing the compiler
    (void) data;
    (void) params;

    // 5 cases:
    // - no channel in the list
    // - no channel with matching name in a list of 1 channel
    // - no channel with matching name in a list of more than 1 channels
    // - one channel with matching name in a list of 1 channel
    // - one channel with matching name in a list of more than 1 channels

    // First case:
    channel **chanls = malloc(sizeof(channel *) * MAX_CHANNEL_COUNT);
    chanl_name = "test";
    chanl_obtained = findChannelByName(chanl_name, chanls);

    munit_assert_null(chanl_obtained);

    // Second case:
    chanls[0] = test1;
    chanl_name = "test";
    chanl_obtained = findChannelByName(chanl_name, chanls);

    munit_assert_null(chanl_obtained);

    // Third case:
    chanls[1] = test2;
    chanl_name = "test";
    chanl_obtained = findChannelByName(chanl_name, chanls);

    munit_assert_null(chanl_obtained);

    // Fourth case:
    chanls[0] = test;
    chanls[1] = NULL;
    chanl_name = "test";
    chanl_obtained = findChannelByName(chanl_name, chanls);

    munit_assert_not_null(chanl_obtained);
    munit_assert_string_equal(chanl_obtained->name, chanl_name);

    // Fifth case:
    chanls[1] = test1;
    chanl_name = "test1";
    chanl_obtained = findChannelByName(chanl_name, chanls);

    munit_assert_not_null(chanl_obtained);
    munit_assert_string_equal(chanl_obtained->name, chanl_name);

    free(test);
    free(test1);
    free(test2);
    free(chanls);
    return MUNIT_OK;
}
#pragma GCC diagnostic pop
