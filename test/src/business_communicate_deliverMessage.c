#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/business/communicate.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_deliverMessage(const MunitParameter params[], void *data) {
    char *message_sent = "I am a message sent through the server!";
    char *expected = "02                      I am a message sent through the server!";
    char *obtained_user1;
    char *obtained_user2;
    char *obtained_user3;
    char *obtained_user4;
    int pipe_fd_user1[2];
    int pipe_fd_user2[2];
    int pipe_fd_user3[2];
    int pipe_fd_user4[2];

    int channel_count;
    channel **chanls = malloc(sizeof(channel *) * MAX_CHANNEL_COUNT);
    channel *chanl;


    // Silencing the compiler
    (void) data;
    (void) params;

    obtained_user1 = malloc(sizeof(char) * NWK_MAX_MESSAGE_LENGTH);
    obtained_user2 = malloc(sizeof(char) * NWK_MAX_MESSAGE_LENGTH);
    obtained_user3 = malloc(sizeof(char) * NWK_MAX_MESSAGE_LENGTH);
    obtained_user4 = malloc(sizeof(char) * NWK_MAX_MESSAGE_LENGTH);

    // For this test, we'll create one channel, having a total of four subscribers. Of course, we
    // are also going to create a pipe for each of them, and after the delivery of the message, we
    // will read each and every, and check that the message has successfully been received.
    pipe(pipe_fd_user1);
    pipe(pipe_fd_user2);
    pipe(pipe_fd_user3);
    pipe(pipe_fd_user4);

    chanl = createChannel("test", chanls, &channel_count);
    subscribeUser(&pipe_fd_user1[1], chanl);
    subscribeUser(&pipe_fd_user2[1], chanl);
    subscribeUser(&pipe_fd_user3[1], chanl);
    subscribeUser(&pipe_fd_user4[1], chanl);

    deliverMessage(message_sent, chanl);

    close(pipe_fd_user1[1]);
    close(pipe_fd_user2[1]);
    close(pipe_fd_user3[1]);
    close(pipe_fd_user4[1]);

    // Getting the output
    read(pipe_fd_user1[0], obtained_user1, NWK_MAX_MESSAGE_LENGTH);
    read(pipe_fd_user2[0], obtained_user2, NWK_MAX_MESSAGE_LENGTH);
    read(pipe_fd_user3[0], obtained_user3, NWK_MAX_MESSAGE_LENGTH);
    read(pipe_fd_user4[0], obtained_user4, NWK_MAX_MESSAGE_LENGTH);

    munit_assert_string_equal(obtained_user1, expected);
    munit_assert_string_equal(obtained_user2, expected);
    munit_assert_string_equal(obtained_user3, expected);
    munit_assert_string_equal(obtained_user4, expected);

    
    free(obtained_user1);
    free(obtained_user2);
    free(obtained_user3);
    free(obtained_user4);
    deleteChannel(chanl, chanls, &channel_count);
    free(chanls);
    return MUNIT_OK;
}
#pragma GCC diagnostic pop
