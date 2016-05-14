#ifndef BETTERSOCKETS_H
#define BETTERSOCKETS_H

/**
 * This file is used to import functions described in bs.c
 * Please refer to bs.c for more info
 * This file contains functions used to abstract low-level network programming
 */

#ifdef __cplusplus
extern "C" {
#endif


#ifdef WIN32
#define OS_WINDOWS
#else
#define OS_UNIX // bad way of checking,... to change later
#endif
//__APPLE__ compatibility to do

#ifdef OS_WINDOWS
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#endif
#ifdef OS_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#endif

int loadsocklib(void);
int freesocklib(void);
int socktcp(int* psocket);
int sockclose(int socket);

int sockconnect(int socket, const char* address, int port);
int socklisten(int socket, const char* address, int port, int n);
int sockaccept(int socket, int* p_c_socket, char* c_address, int* p_c_port);
int hnametoipv4(const char* address, char* ipv4str);

int sendl(int socket, const void* buf, int size, int flags);
int recvl(int socket, void* buf, int recvsize, int flags);

int sockreadable(int socket);


#ifdef __cplusplus
}
#endif

#endif

