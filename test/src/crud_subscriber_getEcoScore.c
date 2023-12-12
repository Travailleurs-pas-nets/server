#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/crud/crud_subscriber.h"
#define GET_ECO_SCORE_TEST_VALUES_COUNT 3

typedef struct {
    unsigned short eco_score;
} GetEcoScoreTestStruct;

GetEcoScoreTestStruct getEcoScoreTestValues[GET_ECO_SCORE_TEST_VALUES_COUNT] = {
    { 0 },      // Normal case
    { 47 },     // Normal case
    { 100 },    // Normal case
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_getEcoScore(const MunitParameter params[], void *data) {
    subscriber *sub = malloc(sizeof(subscriber));

    // Silencing the compiler
    (void) data;
    (void) params;

    for (int i = 0; i < GET_ECO_SCORE_TEST_VALUES_COUNT; i++) {
        sub->ecoScore = getEcoScoreTestValues[i].eco_score;

        unsigned int expected = getEcoScoreTestValues[i].eco_score;
        unsigned int obtained = getEcoScore(sub);

        munit_assert_uint16(obtained, ==, expected);
    }

    free(sub);

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
