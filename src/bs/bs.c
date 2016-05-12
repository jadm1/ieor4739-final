#include "bs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef min
#define min(a, b) ((b > a) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((b > a) ? (b) : (a))
#endif

int hnametoinaddr(const char* address, struct in_addr* p_ipv4);
int inaddrtostr(const struct in_addr* p_ipv4, char* ips, int ips_size);
int inttosaport(const int port, u_short* p_saport);
int saporttoint(const u_short* p_saport, int* p_port);



#ifdef OS_UNIX
int get_ticks_ms(void)
{
	struct tms tm;
	return times(&tm) * 10;
}
#endif
#ifdef OS_WINDOWS
int get_ticks_ms(void)
{
	return (int)GetTickCount();
}
#endif


int loadsocklib()
{
#ifdef OS_WINDOWS
	WSADATA wsaData;
	WORD wsaVer;
	memset(&wsaData, 0, sizeof(WSADATA));
	wsaVer = MAKEWORD(2, 2);
	return WSAStartup(wsaVer, &wsaData);
#endif
#ifdef OS_UNIX
	return 0;
#endif
	return -1;
}

int freesocklib()
{
#ifdef OS_WINDOWS
	return WSACleanup();
#endif
#ifdef OS_UNIX
	return 0;
#endif
	return -1;
}

int socktcp(int* psocket) {
	int sock;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		return -1;
	}

	*psocket = sock;

	return 0;
}

int sockclose(int socket)
{
#ifdef OS_WINDOWS
	int ret = 0;
	closesocket(socket);
	return ret;
#endif
#ifdef OS_UNIX
	return close(socket);
#endif
	return -1;
}

/**
 * This function sends data of a given size
 * Keeps sending data while there is remaining data to be sent according to the size parameter
 *
 * Parameters:
 * int socket
 * socket identifier used for communication
 * const void* buf
 * buffer of the data to be sent
 * sendl does not modify the buffer
 * int size
 * number of bytes to send
 * the buffer should has at least size bytes
 * int flags
 * flags for the send() function
 *
 * Return value:
 * number of bytes sent (should be size if everything went well)
 * if less it means the socket is not connected or no data is available (non blocking mode)
 */
int sendl(int socket, const void* buf, int size, int flags)
{
	int ret;
	const char* p = (const char*)buf;
	int sent = 0;

	while (sent < size)
	{
		ret = send(socket, (const void*)p, size - sent, flags);
		if (ret < 0)
			return -1;
		if (ret == 0) {
			// exit because no data sent (usually means socket disconnected)
			return sent;
		}
		p += ret;
		sent += ret;
	}

	return sent;
}


/**
 * This function receives data until a given size is reached
 * It keeps receiving data while there is remaining data to be received according to the recvsize parameter
 * This function is useful only if we know beforehand the exact size of the message we want to receive
 * All the data will be written in the buffer buf
 *
 * Parameters:
 * int socket
 * socket identifier used for communication
 * void* buf
 * pointer to a buffer used to receive the data
 * int recvsize
 * exact number of bytes to receive.
 * The function will continue receiving until recvsize bytes have been received
 * int flags
 * flags for the recv() function
 *
 * Return value:
 * number of bytes read (should be recvsize if everything went well)
 * if less it means the socket is not connected or no data is available (non blocking mode)
 */
int recvl(int socket, void* buf, int recvsize, int flags)
{
	int ret;
	char* p = (char*)buf;
	int rcvd = 0;

	while (rcvd < recvsize)
	{
		ret = recv(socket, (void*)p, recvsize - rcvd, flags);
		if (ret < 0)
			return -1;
		if (ret == 0) {
			// exit because no data available (usually means socket disconnected)
			return rcvd;
		}
		p += ret;
		rcvd += ret;
	}

	return rcvd;
}


int sockreadable(int socket)
{
#ifdef OS_WINDOWS
	fd_set fds;
	struct timeval timeout;
	timeout.tv_usec = 0;
	timeout.tv_sec  = 0;
	fds.fd_count = 1;
	fds.fd_array[0] = socket;
	if (timeout.tv_sec >= 0 || timeout.tv_sec >= 0)
		return select(0, &fds, NULL, NULL, &timeout);
	else
		return select(0, &fds, NULL, NULL, NULL);
#endif
#ifdef OS_UNIX
	int r;
	fd_set fds;
	struct timeval timeout;
	timeout.tv_usec = 0;
	timeout.tv_sec  = 0;
	FD_ZERO(&fds);
	FD_SET(socket, &fds);
	if (timeout.tv_sec >= 0 || timeout.tv_sec >= 0)
		r = select(socket + 1, &fds, NULL, NULL, &timeout);
	else
		r = select(socket + 1, &fds, NULL, NULL, NULL);

	if (r < 0)
		return -1;

	return FD_ISSET(socket, &fds);
#endif
	return -1;
}


int setreuseaddr(int socket)
{
	int opt = 1;
	int ret;
	ret = setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(int));
	if (ret < 0)
		return -1;
	return 0;
}




int hnametoinaddr(const char* address, struct in_addr* p_ipv4)
{
	struct hostent* rhost = NULL;
	int i;

	if (address == NULL) {
		p_ipv4->s_addr = htonl(0);
		return -1;
	}
	if (address[0] == '\0') {
		p_ipv4->s_addr = htonl(0);
		return -1;
	}

	for (i = 0; i < strlen(address); i++)
	{
		if ((((address[i] < '0') || (address[i] > '9')) && (address[i] != '.')) || (i >= 16))
		{
			rhost = gethostbyname(address);
			if (rhost != NULL) {
				p_ipv4->s_addr = *(u_long*)(rhost->h_addr_list[0]);
				return 0;
			}
			else {
				p_ipv4->s_addr = htonl(0);
				return -1;
			}
		}
	}

	p_ipv4->s_addr = inet_addr(address);
	return 0;
}

int inaddrtostr(const struct in_addr* p_ipv4, char* ips, int ips_size)
{
	strncpy(ips, inet_ntoa(*p_ipv4), ips_size);
	return 0;
}

int inttosaport(const int port, u_short* p_saport) {
	*p_saport = htons((unsigned short)port);
	return 0;
}

int saporttoint(const u_short* p_saport, int* p_port) {
	*p_port = (int)ntohs(*p_saport);
	return 0;
}

int hnametoipv4(const char* address, char* ipv4str) {
	int ret = 0;
	struct in_addr ipv4;

	ret = hnametoinaddr(address, &ipv4);
	if (ret < 0) {
		return -1;
	}

	ret = inaddrtostr(&ipv4, ipv4str, 16);
	if (ret < 0) {
		return -1;
	}

	return 0;
}

int sockconnect(int socket, const char* address, int port) {
	int ret = 0;
	struct sockaddr_in sin_c;

	sin_c.sin_family = AF_INET;
	inttosaport(port, &sin_c.sin_port);
	hnametoinaddr(address, &sin_c.sin_addr);

	ret = connect(socket, (struct sockaddr*)&sin_c, sizeof(struct sockaddr_in));

	return ret;
}

int socklisten(int socket, const char* address, int port, int n) {
	int ret = 0;
	struct sockaddr_in sin_s;

	sin_s.sin_family = AF_INET;
	inttosaport(port, &sin_s.sin_port);
	hnametoinaddr(address, &sin_s.sin_addr);

#ifdef OS_UNIX
	// trick to prevent ports from being unavailable on some systems
	ret = setreuseaddr(socket);
	if (ret < 0) {
		return -1;
	}
#endif

	// bind data structures to a fixed port
	ret = bind(socket, (const struct sockaddr*)&sin_s, sizeof(sin_s));
	if (ret < 0) {
		return -1;
	}

	// listen that is create a small queue in the OS network stack to handle incoming connections
	// does not matter if nbacklog is not large.
	// incoming connections should be accepted as soon as possible
	// a higher level queue and accept mechanisms can be implemented to handle any number of clients
	ret = listen(socket, n);
	if (ret < 0) {
		return -1;
	}

	return ret;
}

int sockaccept(int s_socket, int* p_c_socket, char* c_address, int* p_c_port) {
	int ret = 0;
	int socket_c;
#ifdef OS_WINDOWS
	int accept_sockaddr_len;
#endif
#ifdef OS_UNIX
	unsigned int accept_sockaddr_len;
#endif
	struct sockaddr_in sin_c;

	accept_sockaddr_len = sizeof(struct sockaddr_in);
	socket_c = accept(s_socket, (struct sockaddr*)&sin_c, &accept_sockaddr_len);
	if (socket_c < 0) {
		return -1;
	}

	*p_c_socket = socket_c;

	if (c_address != NULL)
		inaddrtostr(&sin_c.sin_addr, c_address, 16);
	if (p_c_port != NULL)
		saporttoint(&sin_c.sin_port, p_c_port);

	return ret;
}

