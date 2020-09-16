/**
 * process per connection
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

#define errno_abort(text) \
	do {\
		fprintf(stderr, "%s at \"%s\":%d: %s\n",\
			text, __FILE__, __LINE__, strerror(errno));\
		abort();\
	} while(0)
    
int tcp_listen() {}