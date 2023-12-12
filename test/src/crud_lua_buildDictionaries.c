#include "../include/munit.h"
#include "../../src/crud/crud_lua.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
static MunitResult test_buildDictionaries(const MunitParameter params[], void *data) {
    lua_State *state = initialiseLua();

    // Silencing the compiler
    (void) data;
    (void) params;

    // Not much to test here...
    buildDictionaries(state);

    // One assertion, but more by principle than anything else. Basically, if we get there, it means
    // the function succeeded.
    munit_assert_not_null(state);

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
