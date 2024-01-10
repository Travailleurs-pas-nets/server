#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/business/communicate.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_sendMessage(const MunitParameter params[], void *data) {
    char *expected1 = "02                      Je suis un message"; // -1 ecological points
    char *expected2 = "02                      Je suis écologique"; // ±0 ecological points
    char *expected3 = "02                      Je suis un planeur"; // +1 ecological points
    char *obtained1;
    char *obtained2;
    char *obtained3;
    int pipe_fd_subscription[2];
    int pipe_fd_reception[2];

    lua_State *L = initialiseLua();
    buildDictionaries(L);
    int channel_count = 0;
    channel **chanls = malloc(sizeof(channel *) * MAX_CHANNEL_COUNT);
    obtained1 = malloc(sizeof(char) * NWK_MAX_MESSAGE_LENGTH);
    obtained2 = malloc(sizeof(char) * NWK_MAX_MESSAGE_LENGTH);
    obtained3 = malloc(sizeof(char) * NWK_MAX_MESSAGE_LENGTH);

    // Silencing the compiler
    (void) data;
    (void) params;

    pipe(pipe_fd_subscription);
    pipe(pipe_fd_reception);
    
    subscriber *sub = subscribeTo(pipe_fd_subscription[1], "test", chanls, &channel_count);
    close(pipe_fd_subscription[1]);
    close(pipe_fd_subscription[0]);

    sub->transferSocket = &(pipe_fd_reception[1]);

    sendMessage(L, sub, "Je suis un message");
    munit_assert_int32(sub->ecoScore, ==, ECO_BASE-1);
    read(pipe_fd_reception[0], obtained1, NWK_MAX_MESSAGE_LENGTH);
    munit_assert_string_equal(obtained1, expected1);
    
    sendMessage(L, sub, "Je suis écologique");
    munit_assert_int32(sub->ecoScore, ==, ECO_BASE-1);
    read(pipe_fd_reception[0], obtained2, NWK_MAX_MESSAGE_LENGTH);
    munit_assert_string_equal(obtained2, expected2);
    
    sendMessage(L, sub, "Je suis un planeur");
    munit_assert_int32(sub->ecoScore, ==, ECO_BASE);
    read(pipe_fd_reception[0], obtained3, NWK_MAX_MESSAGE_LENGTH);
    munit_assert_string_equal(obtained3, expected3);

    
    free(obtained1);
    free(obtained2);
    free(obtained3);
    free(sub);
    free(chanls);
    return MUNIT_OK;
}
#pragma GCC diagnostic pop
