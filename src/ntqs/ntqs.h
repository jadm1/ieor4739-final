#ifndef NTQS_H
#define NTQS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <utl.h>
#include <bs.h>

/**
 *  This file is only a local header used by the server program for prototypes and internal data structures
 */

typedef struct ntqs_server {
	int verbose;
	int socket;
	char* host_address;
	int host_port;
	int n_queues;
	int n_managers;
	pthread_mutex_t* m_console;
	pthread_mutex_t* m_clients;
	llq* q_clients;
	pthread_mutex_t** m_queues;
	llq** queues;
} ntqs_server;

typedef struct ntqs_manager {
	ntqs_server* ntqs;
	int id;
	pthread_mutex_t* m_interrupt;
	int interrupt;
	pthread_t thread;
} ntqs_manager;


typedef struct ntqs_client {
	int socket;
	char remote_ip[16];
	int remote_port;
} ntqs_client;


extern int signal_interrupt;

int ntqserver(int verbose, char* address, int port, int n_queues, int n_managers);
void* ntqmanagerwrapper(void* params);
int ntqmanager(ntqs_manager* manager);


#ifdef __cplusplus
}
#endif

#endif

