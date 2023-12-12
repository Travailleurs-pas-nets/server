#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/crud/crud_lua.h"
#define REPLACE_POLLUTANT_WORDS_TEST_VALUES_COUNT 23

#define SENTENCE0 "word"
#define SENTENCE1 "only one pollutant word"
#define SENTENCE2 "a"
#define SENTENCE3 "this is a pollutant word"
#define SENTENCE4 "a pollutant word"
#define SENTENCE5 "this is a"
#define SENTENCE6 "this word is a"
#define SENTENCE7 "is a pollutant word"
#define SENTENCE8 "a pollutant word is"

typedef struct {
    char *input;
    char *pollutant_words_indexes;
    int count;
} ReplacePollutantWordsTestStruct;

ReplacePollutantWordsTestStruct replacePollutantWordsTestValues[REPLACE_POLLUTANT_WORDS_TEST_VALUES_COUNT] = {
    { SENTENCE0, "0", 1 },        // Normal case, 1 pollutant word only
    { SENTENCE1, "5", 1 },        // Normal case, 1 pollutant word in the middle
    { SENTENCE1, "0", 1 },        // Normal case, 1 pollutant word at the beginning
    { SENTENCE1, "19", 1 },       // Normal case, 1 pollutant word at the end
    { SENTENCE1, "5_9", 2 },     // Normal case, 2 pollutant words in the middle
    { SENTENCE1, "0_9", 2 },     // Normal case, 2 pollutant words at the beginning and in the middle
    { SENTENCE1, "9_19", 2 },    // Normal case, 2 pollutant words in the middle and at the end
    { SENTENCE1, "0_19", 2 },    // Normal case, 2 pollutant words at the beginning and in the end
    { SENTENCE2, "0", 1 },        // Edge case, 1 pollutant word too short only
    { SENTENCE3, "8", 1 },        // Edge case, 1 pollutant word too short in the middle
    { SENTENCE4, "0", 1 },        // Edge case, 1 pollutant word too short at the beginning
    { SENTENCE5, "0", 1 },        // Edge case, 1 pollutant word too short at the end
    { SENTENCE3, "8_10", 2 },    // Edge case, 1 pollutant word too short in the middle & 1 pollutant word in the middle
    { SENTENCE3, "0_8", 2 },     // Edge case, 1 pollutant word too short in the middle & 1 pollutant word at the beginning
    { SENTENCE3, "8_20", 2 },    // Edge case, 1 pollutant word too short in the middle & 1 pollutant word at the end
    { SENTENCE4, "0_2", 2 },     // Edge case, 1 pollutant word too short at the beginning & 1 pollutant word in the middle
    { SENTENCE4, "0_12", 2 },    // Edge case, 1 pollutant word too short at the beginning & 1 pollutant word at the end
    { SENTENCE6, "5_13", 2 },    // Edge case, 1 pollutant word too short at the end & 1 pollutant word in the middle
    { SENTENCE6, "0_13", 2 },    // Edge case, 1 pollutant word too short at the end & 1 pollutant word in the beginning
    { SENTENCE3, "5_8", 2 },     // Edge case, 2 pollutant words too short in the middle
    { SENTENCE7, "0_5", 2 },     // Edge case, 2 pollutant words too short at the beginning and in the middle
    { SENTENCE5, "5_8", 2 },     // Edge case, 2 pollutant words too short in the middle and at the end
    { SENTENCE8, "0_17", 2 }     // Edge case, 2 pollutant words too short at the beginning and at the end
};

int *make_indexes(int count, ...) {
    va_list arg_list;
    int *indexes = malloc(sizeof(int) * count);
    
    va_start(arg_list, count);
    for (int i = 0; i < count; i++) {
        indexes[i] = va_arg(arg_list, int);
    }
    va_end(arg_list);

    return indexes;
}

int *get_indexes(char *str) {
    if (strcmp(str, "0") == 0) return make_indexes(1, 0);
    if (strcmp(str, "5") == 0) return make_indexes(1, 5);
    if (strcmp(str, "19") == 0) return make_indexes(1, 19);
    if (strcmp(str, "5_9") == 0) return make_indexes(2, 5, 9);
    if (strcmp(str, "0_9") == 0) return make_indexes(2, 0, 9);
    if (strcmp(str, "9_19") == 0) return make_indexes(2, 9, 19);
    if (strcmp(str, "0_19") == 0) return make_indexes(2, 0, 19);
    if (strcmp(str, "8") == 0) return make_indexes(1, 8);
    if (strcmp(str, "8_10") == 0) return make_indexes(2, 8, 10);
    if (strcmp(str, "0_8") == 0) return make_indexes(2, 0, 8);
    if (strcmp(str, "8_20") == 0) return make_indexes(2, 8, 20);
    if (strcmp(str, "0_2") == 0) return make_indexes(2, 0, 2);
    if (strcmp(str, "0_12") == 0) return make_indexes(2, 0, 12);
    if (strcmp(str, "5_13") == 0) return make_indexes(2, 5, 13);
    if (strcmp(str, "0_13") == 0) return make_indexes(2, 0, 13);
    if (strcmp(str, "5_8") == 0) return make_indexes(2, 5, 8);
    if (strcmp(str, "0_5") == 0) return make_indexes(2, 0, 5);
    if (strcmp(str, "0_17") == 0) return make_indexes(2, 0, 17);

    return NULL;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_replacePollutantWords(const MunitParameter params[], void *data) {
    // Silencing the compiler
    (void) data;
    (void) params;

    lua_State *L = initialiseLua();
    buildDictionaries(L);
    
    for (int i = 0; i < REPLACE_POLLUTANT_WORDS_TEST_VALUES_COUNT; i++) {
        int *local_indexes = get_indexes(replacePollutantWordsTestValues[i].pollutant_words_indexes);

        // We are going to duplicate the input strings and make the pointers to the pinned positions.
        char *local_input = strdup(replacePollutantWordsTestValues[i].input);
        char **pollutant_words_pointers = malloc(sizeof(char *) * replacePollutantWordsTestValues[i].count);
        for (int j = 0; j < replacePollutantWordsTestValues[i].count; j++) {
            pollutant_words_pointers[j] = local_input + local_indexes[j];
        }

        // All setup, let's make the call! :D
        replacePollutantWords(L, pollutant_words_pointers, replacePollutantWordsTestValues[i].count);

        unsigned short obtained_word_count;
        unsigned short expected_word_count;
        char **obtained = split(local_input, ' ', &obtained_word_count);
        char **expected = split(replacePollutantWordsTestValues[i].input, ' ', &expected_word_count);

        // First check: length
        int local_index = 0;
        munit_assert_int32(obtained_word_count, ==, expected_word_count);
        for (int j = 0; j < obtained_word_count; j++) {
            bool is_modified = false;

            for (int k = 0; k < replacePollutantWordsTestValues[i].count; k++) {
                is_modified = (local_index == local_indexes[k]);
                if (is_modified) break;
            }

            if (is_modified) {
                munit_assert_string_not_equal(obtained[j], expected[j]);
            } else {
                munit_assert_string_equal(obtained[j], expected[j]);
            }

            local_index += strlen(obtained[j]) + 1; // +1 for the space removed when splitting.
        }



        // freeing the memory
        free(local_indexes);
        free(local_input);
        free(pollutant_words_pointers);
        free(expected);
        free(obtained);
    }

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
