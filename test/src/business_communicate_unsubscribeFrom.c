#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/business/communicate.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_unsubscribeFrom(const MunitParameter params[], void *data) {
    char *expected = "01                      0";
    char *obtained1;
    char *obtained2;
    char *obtained3;
    int pipe_fd_user1[2];
    int pipe_fd_user2[2];
    int pipe_fd_user3[2];
    int pipe_fr_unsub_user1[2];
    int pipe_fr_unsub_user2[2];
    int pipe_fr_unsub_user3[2];

    int channel_count = 0;
    channel **chanls = malloc(sizeof(channel *) * MAX_CHANNEL_COUNT);
    obtained1 = malloc(sizeof(char) * NWK_MAX_MESSAGE_LENGTH);
    obtained2 = malloc(sizeof(char) * NWK_MAX_MESSAGE_LENGTH);
    obtained3 = malloc(sizeof(char) * NWK_MAX_MESSAGE_LENGTH);

    // Silencing the compiler
    (void) data;
    (void) params;

    pipe(pipe_fd_user1);
    pipe(pipe_fd_user2);
    pipe(pipe_fd_user3);
    
    subscriber *sub1 = subscribeTo(pipe_fd_user1[1], "test", chanls, &channel_count);
    subscriber *sub2 = subscribeTo(pipe_fd_user2[1], "test", chanls, &channel_count);
    subscriber *sub3 = subscribeTo(pipe_fd_user3[1], "test", chanls, &channel_count);
    close(pipe_fd_user1[1]);
    close(pipe_fd_user2[1]);
    close(pipe_fd_user3[1]);
    close(pipe_fd_user1[0]);
    close(pipe_fd_user2[0]);
    close(pipe_fd_user3[0]);

    pipe(pipe_fr_unsub_user1);
    pipe(pipe_fr_unsub_user2);
    pipe(pipe_fr_unsub_user3);
    sub1->transferSocket = &(pipe_fr_unsub_user1[1]);
    sub2->transferSocket = &(pipe_fr_unsub_user2[1]);
    sub3->transferSocket = &(pipe_fr_unsub_user3[1]);

    unsubscribeFrom(sub1, "test", chanls, &channel_count);
    unsubscribeFrom(sub2, "test", chanls, &channel_count);
    unsubscribeFrom(sub3, "test", chanls, &channel_count);
    munit_assert_int32(channel_count, ==, 0);
    
    // Getting actual output
    read(pipe_fr_unsub_user1[0], obtained1, NWK_MAX_MESSAGE_LENGTH);
    read(pipe_fr_unsub_user2[0], obtained2, NWK_MAX_MESSAGE_LENGTH);
    read(pipe_fr_unsub_user3[0], obtained3, NWK_MAX_MESSAGE_LENGTH);

    munit_assert_string_equal(obtained1, expected);
    munit_assert_string_equal(obtained2, expected);
    munit_assert_string_equal(obtained3, expected);

    
    free(obtained1);
    free(obtained2);
    free(obtained3);
    free(chanls);
    return MUNIT_OK;
}
#pragma GCC diagnostic pop
