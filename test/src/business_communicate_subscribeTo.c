#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/business/communicate.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_subscribeTo(const MunitParameter params[], void *data) {
    int bufferLength = NWK_OPTION_CODE_LENGTH + 2;
    char *expected = "00                      0";
    char *obtained1;
    char *obtained2;
    char *obtained3;
    char *obtained4;
    char *obtained5;
    char *obtained6;
    char *obtained7;
    char *obtained8;
    char *obtained9;
    char *obtained10;
    int pipe_fd_user1[2];
    int pipe_fd_user2[2];
    int pipe_fd_user3[2];
    int pipe_fd_user4[2];
    int pipe_fd_user5[2];
    int pipe_fd_user6[2];
    int pipe_fd_user7[2];
    int pipe_fd_user8[2];
    int pipe_fd_user9[2];
    int pipe_fd_user10[2];
    int pipe_fd_user11[2];

    int channel_count = 0;
    channel **chanls = malloc(sizeof(channel *) * MAX_CHANNEL_COUNT);
    obtained1 = malloc(sizeof(char) * bufferLength);
    obtained2 = malloc(sizeof(char) * bufferLength);
    obtained3 = malloc(sizeof(char) * bufferLength);
    obtained4 = malloc(sizeof(char) * bufferLength);
    obtained5 = malloc(sizeof(char) * bufferLength);
    obtained6 = malloc(sizeof(char) * bufferLength);
    obtained7 = malloc(sizeof(char) * bufferLength);
    obtained8 = malloc(sizeof(char) * bufferLength);
    obtained9 = malloc(sizeof(char) * bufferLength);
    obtained10 = malloc(sizeof(char) * bufferLength);

    // Silencing the compiler
    (void) data;
    (void) params;

    pipe(pipe_fd_user1);
    pipe(pipe_fd_user2);
    pipe(pipe_fd_user3);
    pipe(pipe_fd_user4);
    pipe(pipe_fd_user5);
    pipe(pipe_fd_user6);
    pipe(pipe_fd_user7);
    pipe(pipe_fd_user8);
    pipe(pipe_fd_user9);
    pipe(pipe_fd_user10);
    pipe(pipe_fd_user11);
    
    subscriber *sub1 = subscribeTo(pipe_fd_user1[1], "test", chanls, &channel_count);
    subscriber *sub2 = subscribeTo(pipe_fd_user2[1], "test", chanls, &channel_count);
    subscriber *sub3 = subscribeTo(pipe_fd_user3[1], "test", chanls, &channel_count);
    subscriber *sub4 = subscribeTo(pipe_fd_user4[1], "test", chanls, &channel_count);
    subscriber *sub5 = subscribeTo(pipe_fd_user5[1], "test", chanls, &channel_count);
    subscriber *sub6 = subscribeTo(pipe_fd_user6[1], "test", chanls, &channel_count);
    subscriber *sub7 = subscribeTo(pipe_fd_user7[1], "test", chanls, &channel_count);
    subscriber *sub8 = subscribeTo(pipe_fd_user8[1], "test", chanls, &channel_count);
    subscriber *sub9 = subscribeTo(pipe_fd_user9[1], "test", chanls, &channel_count);
    subscriber *sub10 = subscribeTo(pipe_fd_user10[1], "test", chanls, &channel_count);
    subscriber *sub11 = subscribeTo(pipe_fd_user11[1], "test", chanls, &channel_count);

    close(pipe_fd_user1[1]);
    close(pipe_fd_user2[1]);
    close(pipe_fd_user3[1]);
    close(pipe_fd_user4[1]);
    close(pipe_fd_user5[1]);
    close(pipe_fd_user6[1]);
    close(pipe_fd_user7[1]);
    close(pipe_fd_user8[1]);
    close(pipe_fd_user9[1]);
    close(pipe_fd_user10[1]);
    close(pipe_fd_user11[1]);

    munit_assert_not_null(sub1);
    munit_assert_not_null(sub2);
    munit_assert_not_null(sub3);
    munit_assert_not_null(sub4);
    munit_assert_not_null(sub5);
    munit_assert_not_null(sub6);
    munit_assert_not_null(sub7);
    munit_assert_not_null(sub8);
    munit_assert_not_null(sub9);
    munit_assert_not_null(sub10);
    munit_assert_null(sub11);
    munit_assert_int32(channel_count, ==, 1);

    // Getting the output
    read(pipe_fd_user1[0], obtained1, bufferLength);
    read(pipe_fd_user2[0], obtained2, bufferLength);
    read(pipe_fd_user3[0], obtained3, bufferLength);
    read(pipe_fd_user4[0], obtained4, bufferLength);
    read(pipe_fd_user5[0], obtained5, bufferLength);
    read(pipe_fd_user6[0], obtained6, bufferLength);
    read(pipe_fd_user7[0], obtained7, bufferLength);
    read(pipe_fd_user8[0], obtained8, bufferLength);
    read(pipe_fd_user9[0], obtained9, bufferLength);
    read(pipe_fd_user10[0], obtained10, bufferLength);

    munit_assert_string_equal(obtained1, expected);
    munit_assert_string_equal(obtained2, expected);
    munit_assert_string_equal(obtained3, expected);
    munit_assert_string_equal(obtained4, expected);
    munit_assert_string_equal(obtained5, expected);
    munit_assert_string_equal(obtained6, expected);
    munit_assert_string_equal(obtained7, expected);
    munit_assert_string_equal(obtained8, expected);
    munit_assert_string_equal(obtained9, expected);
    munit_assert_string_equal(obtained10, expected);

    
    free(obtained1);
    free(obtained2);
    free(obtained3);
    free(obtained4);
    free(obtained5);
    free(obtained6);
    free(obtained7);
    free(obtained8);
    free(obtained9);
    free(obtained10);
    deleteChannel(findChannelByName("test", chanls), chanls, &channel_count);
    free(chanls);
    return MUNIT_OK;
}
#pragma GCC diagnostic pop
