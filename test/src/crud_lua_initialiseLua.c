#include "../include/munit.h"
#include "../../src/crud/crud_lua.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
static MunitResult test_initialiseLua(const MunitParameter params[], void *data) {
    lua_State *obtained;

    // Silencing the compiler
    (void) data;
    (void) params;

    // Not much to test here...
    obtained = initialiseLua();

    // One assertion, but more by principle than anything else. Basically, if we get there, it means
    // the function succeeded.
    munit_assert_not_null(obtained);

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
