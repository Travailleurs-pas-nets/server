// Including µnit.
#include "../include/munit.h"

// Including test functions files.
//#include "network_config_getMachineName.c"

// Defining the list of tests to execute:
static MunitTest test_array[] = {
    //{ (char *) "/network_config/getMachineName         \t\t", test_getMachineName, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    
    // Last item must be this one (it is used to identify that this is the end of the list).
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

// Defining the test suite:
static const MunitSuite test_suite = {
    (char *) "",
    test_array,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, "tpnn", argc, argv);
}
