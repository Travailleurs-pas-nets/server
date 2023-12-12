#include "crud_lua.h"

/**
 * Function to initialise the Lua API.
 */
lua_State *initialiseLua() {
    int executionState;
    lua_State *L = luaL_newstate(); // called L by convention
    luaL_openlibs(L);
    
    executionState = luaL_dofile(L, "./bin/scripts/dictionaries.lua");
    if (executionState != LUA_OK) {
        handleCriticalError("Failed to initialise the Lua API!\n", getTime(), MODE);
    }

    return L;
}

/**
 * This function will build the ecological and pollutant words dictionaries (not talking about the
 * structural pattern here, but the first meaning of a dictionary). It will use the C Lua API,
 * and the dictionaries will be stored in the Lua stack.
 */
void buildDictionaries(lua_State *L) {
    lua_getglobal(L, "create_dictionaries"); // pushes the function to the top of the stack.

    if (!lua_isfunction(L, LUA_STACK_TOP)) {
        handleCriticalError("Failed to find 'create_dictionaries' global function!\n", getTime(), MODE);
    }

    lua_pcall(L, 0, 0, 0);
    // Now the two dictionaries in the script are in the Lua stack, full of words.
}

/**
 * Will replace every pollutant word by ecological words of the same length randomly.
 */
void replacePollutantWords(lua_State *L, char **pollutantWords, int count) {
    for (int i = 0; i < count; i++) {
        const char *word;

        if (wrdlen(pollutantWords[i]) < LUA_MIN_SWAP_LENGTH) {
            // Word shorter than the shortest word in the ecological dictionary
            word = createQuestionMarkString(wrdlen(pollutantWords[i]));
        } else {
            lua_getglobal(L, "swap_for_ecological_word"); // pushes the function to the top of the stack.

            if (!lua_isfunction(L, LUA_STACK_TOP)) {
                handleRuntimeError("Failed to find 'swap_for_ecological_word' global function!\n", getTime(), MODE);
                return;
            }

            // Pushing the parameter to the Lua API
            lua_pushnumber(L, wrdlen(pollutantWords[i]));
            lua_pcall(L, 1, 1, 0);
            word = lua_tostring(L, LUA_STACK_TOP);
        }

        // Replacing the pollutant word's letters one by one.
        for (int j = 0; j < wrdlen(word); j++) {
            pollutantWords[i][j] = word[j];
        }
    }
}
