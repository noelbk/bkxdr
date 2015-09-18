#if OS == OS_WIN32
#include <winsock.h>
#endif

#if OS & OS_UNIX

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>

#include "sock_send_fd.h"
#include "config.h"
#include "itypes.h"

#define SOCK_HAS_AF_UNIX
#define SOCK_SHUTDOWN_RECV 0
#define SOCK_SHUTDOWN_SEND 1

typedef unsigned sockaddrlen_t;
#endif

