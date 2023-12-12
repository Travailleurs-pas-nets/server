#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/crud/crud_subscriber.h"
#define UPDATE_ECO_SCORE_TEST_VALUES_COUNT 4

typedef struct {
    unsigned short eco_score;
    short eco_value;
    unsigned short result;
} UpdateEcoScoreTestStruct;

UpdateEcoScoreTestStruct updateEcoScoreTestValues[UPDATE_ECO_SCORE_TEST_VALUES_COUNT] = {
    { 50, 5, 55 },              // Normal case
    { 50, -5, 45 },             // Normal case
    { 5, -10, 0},               // Edge case
    { ECO_MAX-5, 10, ECO_MAX }  // Edge case
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_updateEcoScore(const MunitParameter params[], void *data) {
    subscriber *sub = malloc(sizeof(subscriber));

    // Silencing the compiler
    (void) data;
    (void) params;

    for (int i = 0; i < UPDATE_ECO_SCORE_TEST_VALUES_COUNT; i++) {
        sub->ecoScore = updateEcoScoreTestValues[i].eco_score;
        updateEcoScore(sub, updateEcoScoreTestValues[i].eco_value);
        
        unsigned int expected = updateEcoScoreTestValues[i].result;
        unsigned int obtained = getEcoScore(sub);

        munit_assert_uint16(obtained, ==, expected);
    }

    free(sub);

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
