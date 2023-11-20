#pragma once

#include "../lua_libs.h"
#include "../homemade_libs.h"
#include "../std_libs.h"

#include "../crud/crud_lua.h"

/**
 * Tells whether the given word is considered pollutant or not.
 * - `true` (1) if it is;
 * - `false` (0) otherwise.
 * 
 * Note: the time complexity of this function in the worst case scenario is O(n), where n is the
 * number of letters in the word to check, not the number of words in the dictionary!
 */
extern bool isWordPollutant(lua_State *L, char *word);

/**
 * Tells whether the given word is considered ecological or not.
 * - `true` (1) if it is;
 * - `false` (0) otherwise.
 * 
 * Note: the time complexity of this function in the worst case scenario is O(n), where n is the
 * number of letters in the word to check, not the number of words in the dictionary!
 */
extern bool isWordEcological(lua_State *L, char *word);

/**
 * Function that will iterate through the words within the message from the client, trying to
 * identify pollutant or ecological words, and editing the content of the three last variables
 * accordingly.
 */
extern void browseForPollutantsAndEcologicalWords(lua_State *L, char **messageWords, 
        char **pollutantWords, int *pollutantWordsCount, int *ecologicalWordsCount);

/**
 * Function that computes the eco-penalty depending on the message length
 */
extern short computeMessageLengthEcoPenalty(int charCount);

/**
 * This function will compute the eco-value of a message. This value will then have to be added to
 * the current eco-value. It may be negative.
 */
extern short computeMessageEcoValue(lua_State *L, char *messageContent, unsigned short current);
