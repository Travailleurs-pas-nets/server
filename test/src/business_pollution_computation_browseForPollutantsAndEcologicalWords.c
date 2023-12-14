#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/business/pollution_computation.h"
#define BROWSE_FOR_POLLUTANTS_AND_ECOLOGICAL_WORDS_TEST_VALUES_COUNT 35

char *test1[2] = { "bonjour", 0 };
char *test2[2] = { "quadrimoteur", 0 };
char *test3[2] = { "vélo", 0 };
char *test4[4] = { "bonjour", "messieurs", "fidèles", 0 };
char *test5[4] = { "quadrimoteur", "messieurs", "fidèles", 0 };
char *test6[4] = { "bonjour", "quadrimoteur", "fidèles", 0 };
char *test7[4] = { "bonjour", "messieurs", "quadrimoteur", 0 };
char *test8[4] = { "vélo", "messieurs", "fidèles", 0 };
char *test9[4] = { "bonjour", "vélo", "fidèles", 0 };
char *test10[4] = { "bonjour", "messieurs", "vélo", 0 };
char *test11[5] = { "quadrimoteur", "plastique", "messieurs", "fidèles", 0 };
char *test12[5] = { "bonjour", "quadrimoteur", "plastique", "fidèles", 0 };
char *test13[5] = { "bonjour", "messieurs", "quadrimoteur", "plastique", 0 };
char *test14[6] = { "bonjour", "quadrimoteur", "messieurs", "fidèles", "plastique", 0 };
char *test15[5] = { "vélo", "fiacre", "messieurs", "fidèles", 0 };
char *test16[5] = { "bonjour", "vélo", "fiacre", "fidèles", 0 };
char *test17[5] = { "bonjour", "messieurs", "vélo", "fiacre", 0 };
char *test18[6] = { "bonjour", "vélo", "messieurs", "fiacre", "fidèles", 0 };
char *test19[6] = { "quadrimoteur", "plastique", "vélo", "fiacre", "fidèles", 0 };
char *test20[6] = { "quadrimoteur", "plastique", "messieurs", "vélo", "fiacre", 0 };
char *test21[6] = { "vélo", "fiacre", "quadrimoteur", "plastique", "fidèles", 0 };
char *test22[6] = { "bonjour", "quadrimoteur", "plastique", "vélo", "fiacre", 0 };
char *test23[6] = { "vélo", "fiacre", "messieurs", "quadrimoteur", "plastique", 0 };
char *test24[6] = { "bonjour", "vélo", "fiacre", "quadrimoteur", "plastique", 0 };
char *test25[8] = { "quadrimoteur", "fiacre", "bonjour", "plastique", "messieurs", "vélo", "fidèles", 0 };
char *test26[3] = { "quadrimoteur", "plastique", 0 };
char *test27[3] = { "vélo", "fiacre", 0 };
char *test28[3] = { "vélo", "quadrimoteur", 0 };
char *test29[3] = { "quadrimoteur", "vélo", 0 };
char *test30[4] = { "bonjour", "vélo", "quadrimoteur", 0 };
char *test31[4] = { "bonjour", "quadrimoteur", "vélo", 0 };
char *test32[4] = { "vélo", "messieurs", "quadrimoteur", 0 };
char *test33[4] = { "quadrimoteur", "messieurs", "vélo", 0 };
char *test34[4] = { "vélo", "quadrimoteur", "fidèles", 0 };
char *test35[4] = { "quadrimoteur", "vélo", "fidèles", 0 };

char *local_pollutant_word[1] = { "quadrimoteur" };
char *local_pollutant_words[2] = { "quadrimoteur", "plastique" };
char *no_pollutant_words[0] = {};

typedef struct {
    char **words;
    char **pollutant_words;
    int pollutant_words_count;
    int ecological_words_count;
} BrowseForPollutantsAndEcologicalWordsTestStruct;

BrowseForPollutantsAndEcologicalWordsTestStruct browseForPollutantsAndEcologicalWordsTestValues[BROWSE_FOR_POLLUTANTS_AND_ECOLOGICAL_WORDS_TEST_VALUES_COUNT] = {
    { test1, no_pollutant_words, 0, 0 },    // Normal case
    { test2, local_pollutant_word, 1, 0 },    // Normal case
    { test3, no_pollutant_words, 0, 1 },    // Normal case
    { test4, no_pollutant_words, 0, 0 },    // Normal case
    { test5, local_pollutant_word, 1, 0 },    // Normal case
    { test6, local_pollutant_word, 1, 0 },    // Normal case
    { test7, local_pollutant_word, 1, 0 },    // Normal case
    { test8, no_pollutant_words, 0, 1 },    // Normal case
    { test9, no_pollutant_words, 0, 1 },    // Normal case
    { test10, no_pollutant_words, 0, 1 },    // Normal case
    { test11, local_pollutant_words, 2, 0 },    // Normal case
    { test12, local_pollutant_words, 2, 0 },    // Normal case
    { test13, local_pollutant_words, 2, 0 },    // Normal case
    { test14, local_pollutant_words, 2, 0 },    // Normal case
    { test15, no_pollutant_words, 0, 2 },    // Normal case
    { test16, no_pollutant_words, 0, 2 },    // Normal case
    { test17, no_pollutant_words, 0, 2 },    // Normal case
    { test18, no_pollutant_words, 0, 2 },    // Normal case
    { test19, local_pollutant_words, 2, 2 },    // Normal case
    { test20, local_pollutant_words, 2, 2 },    // Normal case
    { test21, local_pollutant_words, 2, 2 },    // Normal case
    { test22, local_pollutant_words, 2, 2 },    // Normal case
    { test23, local_pollutant_words, 2, 2 },    // Normal case
    { test24, local_pollutant_words, 2, 2 },    // Normal case
    { test25, local_pollutant_words, 2, 2 },    // Normal case
    { test26, local_pollutant_words, 2, 0 },    // Normal case
    { test27, no_pollutant_words, 0, 2 },    // Normal case
    { test28, local_pollutant_word, 1, 1 },    // Normal case
    { test29, local_pollutant_word, 1, 1 },    // Normal case
    { test30, local_pollutant_word, 1, 1 },    // Normal case
    { test31, local_pollutant_word, 1, 1 },    // Normal case
    { test32, local_pollutant_word, 1, 1 },    // Normal case
    { test33, local_pollutant_word, 1, 1 },    // Normal case
    { test34, local_pollutant_word, 1, 1 },    // Normal case
    { test35, local_pollutant_word, 1, 1 },    // Normal case
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_browseForPollutantsAndEcologicalWords(const MunitParameter params[], void *data) {
    // Silencing the compiler
    (void) data;
    (void) params;


    lua_State *L = initialiseLua();
    buildDictionaries(L);

    for (int i = 0; i < BROWSE_FOR_POLLUTANTS_AND_ECOLOGICAL_WORDS_TEST_VALUES_COUNT; i++) {
        int obtained_pollutant_words_count;
        int obtained_ecological_words_count;
        char **obtained_pollutant_words = malloc(sizeof(char *) * browseForPollutantsAndEcologicalWordsTestValues[i].pollutant_words_count);

        char **expected_pollutant_words = browseForPollutantsAndEcologicalWordsTestValues[i].pollutant_words;
        int expected_pollutant_words_count = browseForPollutantsAndEcologicalWordsTestValues[i].pollutant_words_count;
        int expected_ecological_words_count = browseForPollutantsAndEcologicalWordsTestValues[i].ecological_words_count;

        browseForPollutantsAndEcologicalWords(
            L, browseForPollutantsAndEcologicalWordsTestValues[i].words,
            obtained_pollutant_words,
            &obtained_pollutant_words_count,
            &obtained_ecological_words_count
        );

        munit_assert_int32(obtained_pollutant_words_count, ==, expected_pollutant_words_count);
        munit_assert_int32(obtained_ecological_words_count, ==, expected_ecological_words_count);
        for (int j = 0; j < expected_pollutant_words_count; j++) {
            munit_assert_string_equal(obtained_pollutant_words[j], expected_pollutant_words[j]);
        }

        free(obtained_pollutant_words);
    }

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
