#pragma once

#include "../lua_libs.h"
#include "../homemade_libs.h"

/**
 * Function to initialise the Lua API.
 */
extern lua_State *initialiseLua();

/**
 * This function will build the ecological and pollutant words dictionaries (not talking about the
 * structural pattern here, but the first meaning of a dictionary). It will use the C Lua API,
 * and the dictionaries will be stored in the Lua stack.
 */
extern void buildDictionaries(lua_State *L);

/**
 * Will replace every pollutant word by ecological words of the same length randomly.
 */
extern void replacePollutantWords(lua_State *L, char **pollutantWords, int count);


