// Including µnit.
#include "../include/munit.h"

// Including test functions files.
#include "crud_socket_configureConnectionSocket.c"
#include "crud_lua_initialiseLua.c"
#include "crud_lua_buildDictionaries.c"
#include "crud_lua_replacePollutantWords.c"
#include "crud_channel_initialiseChannelSubscribers.c"
#include "crud_channel_findChannelByName.c"
#include "crud_channel_createChannel.c"
#include "crud_channel_deleteChannelSubscribers.c"
#include "crud_channel_deleteChannel.c"
#include "crud_channel_findOrCreateChannel.c"
#include "crud_subscriber_getEcoScore.c"
#include "crud_subscriber_updateEcoScore.c"
#include "crud_subscriber_subscribeUser.c"
#include "crud_subscriber_unsubscribeUser.c"
#include "business_pollution_computation_isWordPollutant.c"
#include "business_pollution_computation_isWordEcological.c"
#include "business_pollution_computation_browseForPollutantsAndEcologicalWords.c"
#include "business_pollution_computation_computeMessageLengthEcoPenalty.c"
#include "business_pollution_computation_computeMessageEcoValue.c"
#include "business_communicate_notifySubscriptionSuccess.c"
#include "business_communicate_notifyUnsubscriptionSuccess.c"
#include "business_communicate_deliverMessage.c"
#include "business_communicate_subscribeTo.c"
#include "business_communicate_unsubscribeFrom.c"
#include "business_communicate_sendMessage.c"

// Defining the list of tests to execute:
static MunitTest test_array[] = {
    { (char *) "/crud/socket/configureConnectionSocket                               \t\t", test_configureConnectionSocket, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/lua/initialiseLua                                              \t\t", test_initialiseLua, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/lua/buildDictionaries                                          \t\t", test_buildDictionaries, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/lua/replacePollutantWords                                      \t\t", test_replacePollutantWords, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/channel/initialiseChannelSubscribers                           \t\t", test_initialiseChannelSubscribers, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/channel/findChannelByName                                      \t\t", test_findChannelByName, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/channel/createChannel                                          \t\t", test_createChannel, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/channel/deleteChannelSubscribers                               \t\t", test_deleteChannelSubscribers, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/channel/deleteChannel                                          \t\t", test_deleteChannel, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/channel/findOrCreateChannel                                    \t\t", test_findOrCreateChannel, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/channel/getEcoScore                                            \t\t", test_getEcoScore, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/channel/updateEcoScore                                         \t\t", test_updateEcoScore, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/channel/subscribeUser                                          \t\t", test_subscribeUser, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/crud/channel/unsubscribeUser                                        \t\t", test_unsubscribeUser, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/business/pollution_computation/isWordPollutant                      \t\t", test_isWordPollutant, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/business/pollution_computation/isWordEcological                     \t\t", test_isWordEcological, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/business/pollution_computation/browseForPollutantsAndEcologicalWords\t\t", test_browseForPollutantsAndEcologicalWords, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/business/pollution_computation/computeMessageLengthEcoPenalty       \t\t", test_computeMessageLengthEcoPenalty, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/business/pollution_computation/computeMessageEcoValue               \t\t", test_computeMessageEcoValue, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/business/communicate/notifySubscriptionSuccess                      \t\t", test_notifySubscriptionSuccess, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/business/communicate/notifyUnsubscriptionSuccess                    \t\t", test_notifyUnsubscriptionSuccess, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/business/communicate/deliverMessage                                 \t\t", test_deliverMessage, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/business/communicate/subscribeTo                                    \t\t", test_subscribeTo, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/business/communicate/unsubscribeFrom                                \t\t", test_unsubscribeFrom, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *) "/business/communicate/sendMessage                                    \t\t", test_sendMessage, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    
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
    return munit_suite_main(&test_suite, "server", argc, argv);
}
