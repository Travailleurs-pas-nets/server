#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/munit.h"
#include "../../src/business/pollution_computation.h"
#define COMPUTE_MESSAGE_ECO_VALUE_TEST_VALUES_COUNT 20

typedef struct {
    char *message;
    unsigned short current_eco_value;
    short result;
} ComputeMessageEcoValueTestStruct;

ComputeMessageEcoValueTestStruct computeMessageEcoValueTestValues[COMPUTE_MESSAGE_ECO_VALUE_TEST_VALUES_COUNT] = {
    { "je mange du pain", ECO_BASE, 0 },
    { "je mange du pain, le tout dans un long tract qui fait plus de cent-cinquante caractères. À partir de ce point, c'est seulement quarante caractères supplémentaires qu'il me faut.", ECO_BASE, -1 },
    { "je mange du pain, le tout dans un long tract qui fait plus de cent-cinquante caractères. À partir de ce point, c'est seulement quarante caractères supplémentaires qu'il me faut. je mange du pain, le tout dans un long tract qui fait plus de cent-cinquante caractères. À partir de ce point, c'est seulement quarante caractères supplémentaires qu'il me faut.", ECO_BASE, -2 },
    { "je mange du pain, le tout dans un long tract qui fait plus de cent-cinquante caractères. À partir de ce point, c'est seulement quarante caractères supplémentaires qu'il me faut. je mange du pain, le tout dans un long tract qui fait plus de cent-cinquante caractères. À partir de ce point, c'est seulement quarante caractères supplémentaires qu'il me faut. je mange du pain, le tout dans un long tract qui fait plus de cent-cinquante caractères. À partir de ce point, c'est seulement quarante caractères supplémentaires qu'il me faut. je mange du pain, le tout dans un long tract qui fait plus de cent-cinquante caractères. À partir de ce point, c'est seulement quarante caractères supplémentaires qu'il me faut.", ECO_BASE, -3 },
    { "électrifications , électrifications , électrifications !", ECO_BASE, 3 },
    { "électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications !", ECO_BASE, 11 },
    { "électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications !", ECO_BASE, 13 },
    { "électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications ! électrifications , électrifications , électrifications !", ECO_BASE, 12 },
    { "les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source !", ECO_BASE, -1 },
    { "les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source ! les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source !", ECO_BASE, -3 },
    { "les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source ! les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source ! les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source ! les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source !", ECO_BASE, -6 },
    { "les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source ! les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source ! les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source ! les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source ! les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source ! les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source ! les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source ! les bureaux climatisés c'est le futur ! Il nous faut tout miser sur l'avenir, cela coule de source !", ECO_BASE, -11 },
    { "les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source !", ECO_BASE, 1 },
    { "les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source ! les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source !", ECO_BASE, 1 },
    { "les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source ! les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source ! les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source ! les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source !", ECO_BASE, 2 },
    { "les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source ! les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source ! les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source ! les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source ! les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source ! les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source ! les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source ! les locaux climatisés c'est le futur ! Il nous faut tout miser sur la géothermie cela coule de source !", ECO_BASE, 5 },
    { "fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée !", ECO_BASE, -ECO_BASE },
    { "fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée !", ECO_BASE, -ECO_BASE },
    { "fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée !", ECO_BASE, -ECO_BASE },
    { "fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée ! fusée !", ECO_BASE, -ECO_BASE },
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
static MunitResult test_computeMessageEcoValue(const MunitParameter params[], void *data) {
    // Silencing the compiler
    (void) data;
    (void) params;

    lua_State *L = initialiseLua();
    buildDictionaries(L);

    for (int i = 0; i < COMPUTE_MESSAGE_ECO_VALUE_TEST_VALUES_COUNT; i++) {
        short expected = computeMessageEcoValueTestValues[i].result;
        short obtained = computeMessageEcoValue(L, computeMessageEcoValueTestValues[i].message, computeMessageEcoValueTestValues[i].current_eco_value);

        munit_assert_int16(obtained, ==, expected);
    }

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
