#include "pollution_computation.h"

/**
 * Tells whether the given word is considered pollutant or not.
 * - `true` (1) if it is;
 * - `false` (0) otherwise.
 * 
 * Note: the time complexity of this function in the worst case scenario is O(n), where n is the
 * number of letters in the word to check, not the number of words in the dictionary!
 */
bool isWordPollutant(lua_State *L, char *word) {
    lua_getglobal(L, "is_word_pollutant"); // pushes the function to the top of the stack.

    if (!lua_isfunction(L, LUA_STACK_TOP)) {
        handleRuntimeError("Failed to find 'is_word_pollutant' global function!\n", getTime(), MODE);
        return false;
    }

    // Pushing the parameter to the Lua API
    lua_pushstring(L, word);
    lua_pcall(L, 1, 1, 0);
    return lua_toboolean(L, LUA_STACK_TOP); // Retrieving the returned value from the Lua stack
}

/**
 * Tells whether the given word is considered ecological or not.
 * - `true` (1) if it is;
 * - `false` (0) otherwise.
 * 
 * Note: the time complexity of this function in the worst case scenario is O(n), where n is the
 * number of letters in the word to check, not the number of words in the dictionary!
 */
bool isWordEcological(lua_State *L, char *word) {
    lua_getglobal(L, "is_word_ecological"); // pushes the function to the top of the stack.

    if (!lua_isfunction(L, LUA_STACK_TOP)) {
        handleRuntimeError("Failed to find 'is_word_ecological' global function!\n", getTime(), MODE);
        return false;
    }

    // Pushing the parameter to the Lua API
    lua_pushstring(L, word);
    lua_pcall(L, 1, 1, 0);
    return lua_toboolean(L, LUA_STACK_TOP); // Retrieving the returned value from the Lua stack.
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
/**
 * Function that will iterate through the words within the message from the client, trying to
 * identify pollutant or ecological words, and editing the content of the three last variables
 * accordingly.
 */
void browseForPollutantsAndEcologicalWords(lua_State *L, char **messageWords, 
        char **pollutantWords, int *pollutantWordsCount, int *ecologicalWordsCount) {
    int localPollutantWordsCount = 0;
    int localEcologicalWordsCount = 0;

    while (*messageWords != 0) {
        if (isWordPollutant(L, *messageWords)) {
            pollutantWords[localPollutantWordsCount] = *messageWords;
            localPollutantWordsCount++;
        } else if (isWordEcological(L, *messageWords)) {
            localEcologicalWordsCount++;
        }

        ++messageWords;
    }

    *pollutantWordsCount = localPollutantWordsCount;
    *ecologicalWordsCount = localEcologicalWordsCount;
}
#pragma GCC diagnostic pop

/**
 * Function that computes the eco-penalty depending on the message length
 */
short computeMessageLengthEcoPenalty(int charCount) {
    if (charCount <= ECO_LENGTH_PENALTY_0_BOUNDARY) {
        return 0;
    } else if (charCount <= ECO_LENGTH_PENALTY_1_BOUNDARY) {
        return 1;
    } else if (charCount <= ECO_LENGTH_PENALTY_2_BOUNDARY) {
        return 2;
    } else {
        return 3;
    }
}

/**
 * This function will compute the eco-value of a message. This value will then have to be added to
 * the current eco-value. It may be negative.
 */
short computeMessageEcoValue(lua_State *L, char *messageContent, unsigned short current) {
    char **messageWords;
    char **pollutantWords;
    unsigned short wordCount;
    int deltaPollutantAndEcologicalWords;
    int pollutantWordsCount = 0;
    int ecologicalWordsCount = 0;

    debug("[INFO] %s: Computing eco-value for the message from '%s'\n", getTime(), "???", MODE);

    messageWords = split(messageContent, ' ', &wordCount);


    pollutantWords = malloc(sizeof(char *) * wordCount);
    browseForPollutantsAndEcologicalWords(
        L,
        messageWords,
        pollutantWords,
        &pollutantWordsCount,
        &ecologicalWordsCount
    );
    deltaPollutantAndEcologicalWords = ecologicalWordsCount - pollutantWordsCount;

    if ((-1) * deltaPollutantAndEcologicalWords > ECO_MAX_POLLUTANT_WORDS_TOLERATED) {
        replacePollutantWords(L, pollutantWords, pollutantWordsCount);
        return ((-1) * current);
    }

    free(pollutantWords);
    for (int i = 0; i < wordCount; i++) {
        free(messageWords[i]);
    }
    free(messageWords);

    /* We want to cap the increase at the same value as we tolerate pollutant words. */
    return (deltaPollutantAndEcologicalWords > ECO_MAX_POLLUTANT_WORDS_TOLERATED?
                ECO_MAX_POLLUTANT_WORDS_TOLERATED: deltaPollutantAndEcologicalWords
        ) - computeMessageLengthEcoPenalty(strlen(messageContent));
}
