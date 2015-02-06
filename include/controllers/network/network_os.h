#ifndef INCL_NETWORKOS
#define INCL_NETWORKOS

/** \brief Adapted from Netport
 *
 * see https://github.com/Meisaka/NetPort/
 */


#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>

#if defined(_MSC_VER)
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <inaddr.h>
#include <in6addr.h>
#include <mstcpip.h>
#define SHUT_RDWR SD_BOTH
typedef int socklen_t;
#pragma comment(lib, "ws2_32.lib")
#undef SetPort
#else
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netdb.h>
typedef unsigned int socklen_t;
#define INVALID_SOCKET (-1)
#endif

#endif

