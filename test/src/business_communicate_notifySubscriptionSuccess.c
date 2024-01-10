#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/business/communicate.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_notifySubscriptionSuccess(const MunitParameter params[], void *data) {
    char *obtained = malloc(sizeof(char) * (NWK_OPTION_CODE_LENGTH+2));
    char *expected = "00                      0";
    int pipe_fd[2];

    // Silencing the compiler
    (void) data;
    (void) params;

    // Creating a pipe in which the tested function will write.
    pipe(pipe_fd);

    notifySubscriptionSuccess(pipe_fd[1]);
    close(pipe_fd[1]);

    // Getting the output
    read(pipe_fd[0], obtained, NWK_OPTION_CODE_LENGTH+2);
    munit_assert_string_equal(obtained, expected);

    free(obtained);
    return MUNIT_OK;
}
#pragma GCC diagnostic pop
