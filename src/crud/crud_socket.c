#include "crud_socket.h"

/**
 * This function configures the server's connection socket. It creates an IPv4 TCP socket, and
 * binds it, before to return the socket descriptor.
 */
int configureConnectionSocket(sockaddr_in localAddress) {
    int connectionSocketDescriptor;
    int operationResultCode;

    connectionSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (connectionSocketDescriptor < 0) {
        handleCriticalError("Failed to create the connection socket\n", getTime(), MODE);
    }

    operationResultCode = bind(
        connectionSocketDescriptor,
        (sockaddr *)&localAddress,
        sizeof(localAddress)
    );
    if (operationResultCode < 0) {
        handleCriticalError("Failed to bind the socket to the connection address\n", getTime(), MODE);
    }

    return connectionSocketDescriptor;
}
