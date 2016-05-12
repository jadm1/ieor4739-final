#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "ntqs.h"

#ifdef WIN32
#define sigset signal
#endif
void (*sigset(int sig, void (*disp)(int)))(int);
void handlesigint(int i);

int signal_interrupt;


/**
 * This is called when the main thread is interrupted
 * so signal_interrupt is a non shared variable that belongs only to the main thread
 * manager threads have their own interrupt boolean that will be set on exiting
 */
void handlesigint(int signal)
{
	if (!signal_interrupt) {
		signal_interrupt = 1;
	}
}


int main(int argc, char** argv) {
	int ret = 0;
	int i;

	char *address;
	int port;
	int n_queues;
	int n_managers;
	int parse_error;
	int verbose;


	// setup interrupt handler
	signal_interrupt = 0;
	sigset(SIGINT, &handlesigint);


	// Default parameters
	address = "0.0.0.0";
	port = 12345;
	n_queues = 1;
	n_managers = 1;
	verbose = 0;

	parse_error = 0;
	if (argc <= 0){
		parse_error = 1;
	}
	else {
		for (i = 1; i < argc; i++){
			if (!strcmp(argv[i], "-h")) {
				parse_error = 1;
			}
			else if (!strcmp(argv[i], "-v")) {
				verbose = 1;
			}
			else if (!strcmp(argv[i], "-a")) {
				i += 1;
				address = argv[i];
			}
			else if (!strcmp(argv[i], "-p")) {
				i += 1;
				port = atoi(argv[i]);
			}
			else if (!strcmp(argv[i], "-q")) {
				i += 1;
				n_queues = atoi(argv[i]);
			}
			else if (!strcmp(argv[i], "-m")) {
				i += 1;
				n_managers = atoi(argv[i]);
			}
			else {
				fprintf(stderr, "main(): error bad option %s\n", argv[i]);
				parse_error = 1;
			}
		}
	}
	if (parse_error) {
		printf("Usage: %s [-a address] [-p port] [-q number of task queues] [-m number of managers]\n", argv[0]);
		return -1;
	}


	ret = loadsocklib();
	if (ret < 0) {
		fprintf(stderr, "main(): error cannot start sockets library\n");
		return -1;
	}


	ret = ntqserver(verbose, address, port, n_queues, n_managers);
	if (ret < 0) {
		fprintf(stderr, "main(): error in server()\n");
		freesocklib();
		return -1;
	}

	ret = freesocklib();
	if (ret < 0) {
		return -1;
	}

	return 0;
}

