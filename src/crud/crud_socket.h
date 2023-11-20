#pragma once

#include "../std_libs.h"
#include "../homemade_libs.h"

/**
 * This function configures the server's connection socket. It creates an IPv4 TCP socket, and
 * binds it, before to return the socket descriptor.
 */
extern int configureConnectionSocket(sockaddr_in localAddress);
