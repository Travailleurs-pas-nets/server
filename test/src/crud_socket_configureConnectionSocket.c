#include "../include/munit.h"
#include "../../src/crud/crud_socket.h"
#include "../../include/tpnn.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
static MunitResult test_configureConnectionSocket(const MunitParameter params[], void *data) {
    int socket_descriptor;
    sockaddr_in local_address;

    hostent *host = retrieveHost(LL_DEBUG);
    local_address = configureLocalAddress(host, 8000, NWK_SERVER);

    // Silencing the compiler
    (void) data;
    (void) params;

    // Not much to test here...
    socket_descriptor = configureConnectionSocket(local_address);

    // One assertion, but more by principle than anything else. Basically, if we get there, it means
    // the function succeeded.
    munit_assert_int32(socket_descriptor, !=, 0);
    close(socket_descriptor);

    return MUNIT_OK;
}
#pragma GCC diagnostic pop
