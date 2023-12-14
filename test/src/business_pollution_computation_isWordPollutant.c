#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/business/pollution_computation.h"
#define IS_WORD_POLLUTANT_TEST_VALUES_COUNT 2

typedef struct {
    char *word;
    bool is_pollutant;
} IsWordPollutantTestStruct;

IsWordPollutantTestStruct isWordPollutantTestValues[IS_WORD_POLLUTANT_TEST_VALUES_COUNT] = {
    { "clavier", false },       // Normal case
    { "camionnette", true },    // Normal case
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_isWordPollutant(const MunitParameter params[], void *data) {
    // Silencing the compiler
    (void) data;
    (void) params;


    lua_State *L = initialiseLua();
    buildDictionaries(L);

    for (int i = 0; i < IS_WORD_POLLUTANT_TEST_VALUES_COUNT; i++) {
        bool expected = isWordPollutantTestValues[i].is_pollutant;
        bool obtained = isWordPollutant(L, isWordPollutantTestValues[i].word);

        if (expected) munit_assert_true(obtained);
        if (!expected) munit_assert_false(obtained);
    }

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
