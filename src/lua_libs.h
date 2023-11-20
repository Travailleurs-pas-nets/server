#pragma once

/** File to include all Lua libraries & to define Lua constants */

/** Lua headers */
#include "../include/lua.h"            /* Lua lib, allowing to incorporate Lua code to C programs */
#include "../include/lauxlib.h"
#include "../include/lualib.h"

/** Lua constants */
#define LUA_STACK_TOP -1
#define LUA_MIN_SWAP_LENGTH 3
