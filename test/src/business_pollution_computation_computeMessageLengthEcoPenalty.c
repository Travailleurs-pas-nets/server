#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/business/pollution_computation.h"
#define COMPUTE_MESSAGE_LENGTH_ECO_PENALTY_TEST_VALUES_COUNT 9

typedef struct {
    int length;
    short penalty;
} ComputeMessageLengthEcoPenaltyTestStruct;

ComputeMessageLengthEcoPenaltyTestStruct computeMessageLengthEcoPenaltyTestValues[COMPUTE_MESSAGE_LENGTH_ECO_PENALTY_TEST_VALUES_COUNT] = {
    { ECO_LENGTH_PENALTY_0_BOUNDARY-1, 0 }, //Edge case
    { ECO_LENGTH_PENALTY_0_BOUNDARY,   0 }, //Edge case
    { ECO_LENGTH_PENALTY_0_BOUNDARY+1, 1 }, //Edge case
    { ECO_LENGTH_PENALTY_1_BOUNDARY-1, 1 }, //Edge case
    { ECO_LENGTH_PENALTY_1_BOUNDARY,   1 }, //Edge case
    { ECO_LENGTH_PENALTY_1_BOUNDARY+1, 2 }, //Edge case
    { ECO_LENGTH_PENALTY_2_BOUNDARY-1, 2 }, //Edge case
    { ECO_LENGTH_PENALTY_2_BOUNDARY,   2 }, //Edge case
    { ECO_LENGTH_PENALTY_2_BOUNDARY+1, 3 }, //Edge case
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_computeMessageLengthEcoPenalty(const MunitParameter params[], void *data) {
    // Silencing the compiler
    (void) data;
    (void) params;

    for (int i = 0; i < COMPUTE_MESSAGE_LENGTH_ECO_PENALTY_TEST_VALUES_COUNT; i++) {
        short expected = computeMessageLengthEcoPenaltyTestValues[i].penalty;
        short obtained = computeMessageLengthEcoPenalty(computeMessageLengthEcoPenaltyTestValues[i].length);

        munit_assert_int16(obtained, ==, expected);
    }

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
