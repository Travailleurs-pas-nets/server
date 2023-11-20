#pragma once

/** File to include all standard libraries */

#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>    /* For the sockets */
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h> 		    /* For hostent, servent */
#include <string.h> 		/* For bcopy, ... */  
#include <time.h>           /* To get the date and time */
#include <math.h>           /* To get pow() and floor() functions */
#include <stdbool.h>        /* For booleans...... */
#include <stdarg.h>         /* For variadic functions */
#include <pthread.h>        /* For threads */

/** Defining the structures */
typedef struct sockaddr sockaddr;
typedef struct servent servent;
typedef struct sockaddr_in sockaddr_in;
