#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ntqs.h"

/*
 * This is the main thread accepting connections to the server
 */
int ntqserver(int verbose, char* address, int port, int n_queues, int n_managers) {
	int ret = 0;
	int i;
	ntqs_server* ntqs;
	ntqs_manager* managers;
	ntqs_manager* manager;
	pthread_mutex_t m_console_data;
	pthread_mutex_t m_clients_data;
	pthread_mutex_t* m_queues_array;
	pthread_mutex_t* m_interrupt_array;
	ntqs_client* client = NULL;


	printf("Starting server ...\n");

	ntqs = (ntqs_server*)malloc(sizeof(ntqs_server));
	if (ntqs == NULL) {
		fprintf(stderr, "ntqserver(): error not enough memory\n");
		return -1;
	}

	ntqs->verbose = verbose;
	ntqs->host_address = address;
	ntqs->host_port = port;
	ntqs->n_queues = n_queues;
	ntqs->n_managers = n_managers;
	ntqs->socket = 0;
	ntqs->m_console = NULL;
	ntqs->m_clients = NULL;
	ntqs->q_clients = NULL;
	ntqs->m_queues = NULL;
	ntqs->queues = NULL;

	// creating client queue mutex
	ntqs->m_clients = &m_clients_data;
	ret = pthread_mutex_init(ntqs->m_clients, NULL);
	if (ret != 0) {
		fprintf(stderr, "ntqserver(): error mutex creation failed\n");
		return -1;
	}
	// creating client queue
	ret = llq_create(&ntqs->q_clients);
	if (ret != 0) {
		fprintf(stderr, "ntqserver(): error queue creation failed\n");
		return -1;
	}



	// creating queues mutexes
	ntqs->m_queues = (pthread_mutex_t**)malloc(ntqs->n_queues * sizeof(pthread_mutex_t*));
	if (ntqs->m_queues == NULL) {
		fprintf(stderr, "ntqserver(): error not enough memory\n");
		return -1;
	}
	m_queues_array = (pthread_mutex_t*)malloc(ntqs->n_queues * sizeof(pthread_mutex_t));
	if (m_queues_array == NULL) {
		fprintf(stderr, "ntqserver(): error not enough memory\n");
		return -1;
	}
	for (i = 0; i < ntqs->n_queues; i++) {
		ntqs->m_queues[i] = &m_queues_array[i];
		ret = pthread_mutex_init(ntqs->m_queues[i], NULL);
		if (ret != 0) {
			fprintf(stderr, "ntqserver(): error mutex creation failed\n");
			return -1;
		}
	}
	// creating queues
	ntqs->queues = (llq**)malloc(ntqs->n_queues * sizeof(llq*));
	if (ntqs->queues == NULL) {
		fprintf(stderr, "ntqserver(): error not enough memory\n");
		return -1;
	}
	for (i = 0; i < ntqs->n_queues; i++) {
		ret = llq_create(&ntqs->queues[i]);
		if (ret != 0) {
			fprintf(stderr, "ntqserver(): error queue creation failed\n");
			return -1;
		}
	}


	ret = socktcp(&ntqs->socket);
	if (ret < 0) {
		fprintf(stderr, "ntqserver(): error socket creation failed\n");
		return -1;
	}

	ret = socklisten(ntqs->socket, ntqs->host_address, ntqs->host_port, 10);
	if (ret < 0) {
		fprintf(stderr, "ntqserver(): error socket listen failed\n");
		return -1;
	}

	printf("Listening on %s:%d ...\n", ntqs->host_address, ntqs->host_port);


	printf("Starting manager(s) ...\n");
	// setup multithreading

	// create console mutex
	ntqs->m_console = &m_console_data;
	ret = pthread_mutex_init(ntqs->m_console, NULL);
	if (ret != 0) {
		fprintf(stderr, "ntqserver(): error console mutex creation failed\n");
		return -1;
	}

	// create interrupt mutexes array
	m_interrupt_array = (pthread_mutex_t*)malloc(ntqs->n_managers*sizeof(pthread_mutex_t));
	if (m_interrupt_array == NULL) {
		pthread_mutex_lock(ntqs->m_console);
		fprintf(stderr, "ntqserver(): error not enough memory\n");
		pthread_mutex_unlock(ntqs->m_console);
		return -1;
	}


	// create managers
	managers = (ntqs_manager*)malloc(ntqs->n_managers*sizeof(ntqs_manager));
	if (managers == NULL) {
		pthread_mutex_lock(ntqs->m_console);
		fprintf(stderr, "ntqserver(): error not enough memory\n");
		pthread_mutex_unlock(ntqs->m_console);
		return -1;
	}


	for (i = 0; i < ntqs->n_managers; i++) {
		manager = &managers[i];
		manager->ntqs = ntqs;
		manager->id = i;
		manager->m_interrupt = &m_interrupt_array[i];
		manager->interrupt = 0;

		ret = pthread_mutex_init(manager->m_interrupt, NULL);
		if (ret != 0) {
			pthread_mutex_lock(ntqs->m_console);
			fprintf(stderr, "ntqserver(): error interrupt mutex creation failed\n");
			pthread_mutex_unlock(ntqs->m_console);
			return -1;
		}

		ret = pthread_create(&manager->thread, NULL, &ntqmanagerwrapper, (void*)manager);
		if (ret != 0) {
			pthread_mutex_lock(ntqs->m_console);
			fprintf(stderr, "ntqserver(): error thread creation failed\n");
			pthread_mutex_unlock(ntqs->m_console);
			return -1;
		}
	}


	pthread_mutex_lock(ntqs->m_console);
	printf("Accepting connections ...\n");
	pthread_mutex_unlock(ntqs->m_console);

	while (1) {
		pthread_mutex_lock(ntqs->m_console);
		fflush(stderr);
		fflush(stdout);
		pthread_mutex_unlock(ntqs->m_console);

		// create new client data structure
		client = (ntqs_client*)malloc(sizeof(ntqs_client));
		if (client == NULL) {
			pthread_mutex_lock(ntqs->m_console);
			fprintf(stderr, "ntqserver(): error not enough memory\n");
			pthread_mutex_unlock(ntqs->m_console);
			return -1;
		}

		// check for server socket activity
		ret = sockreadable(ntqs->socket);
		if (ret < 0) {
			pthread_mutex_lock(ntqs->m_console);
			fprintf(stderr, "ntqserver(): error socket accept failed\n");
			pthread_mutex_unlock(ntqs->m_console);
			return -1;
		}

		if (ret == 0) {
			// socket idle (no connection)
			UTLsleep(10); // nothing to do so go to sleep
			// check for interrupt signal after sleeping (likely to happen while sleeping)
			if (signal_interrupt) {
				break;
			}
		}
		else {
			// activity on socket -> accept new connection
			ret = sockaccept(ntqs->socket, &client->socket, client->remote_ip, &client->remote_port);
			if (ret < 0) {
				pthread_mutex_lock(ntqs->m_console);
				fprintf(stderr, "ntqserver(): error socket accept failed\n");
				pthread_mutex_unlock(ntqs->m_console);
				return -1;
			}

			pthread_mutex_lock(ntqs->m_console);
			printf("%s:%d connected\n", client->remote_ip, client->remote_port);
			pthread_mutex_unlock(ntqs->m_console);


			//pthread_mutex_lock(ntqs->m_console);
			//printf("%s:%d in queue\n", client->remote_ip, client->remote_port);
			//pthread_mutex_unlock(ntqs->m_console);

			// from there on the client data becomes the property of the manager (unavalaible from the main thread)
			pthread_mutex_lock(ntqs->m_clients);
			llq_push(ntqs->q_clients, (void*)client);
			pthread_mutex_unlock(ntqs->m_clients);
		}
	}


	pthread_mutex_lock(ntqs->m_console);
	printf("Stopping server ...\n");
	pthread_mutex_unlock(ntqs->m_console);

	if (signal_interrupt) {
		for (i = 0; i < ntqs->n_managers; i++) {
			manager = &managers[i];
			pthread_mutex_lock(manager->m_interrupt);
			manager->interrupt = 1;
			pthread_mutex_unlock(manager->m_interrupt);
		}
	}


	for (i = 0; i < ntqs->n_managers; i++) {
		manager = &managers[i];
		ret = pthread_join(manager->thread, NULL);
		ret = pthread_mutex_destroy(manager->m_interrupt);
	}

	free((void*)managers);
	free((void*)m_interrupt_array);
	ret = pthread_mutex_destroy(ntqs->m_console);
	for (i = 0; i < ntqs->n_queues; i++) {
		while (!llq_isempty(ntqs->queues[i])) {
			client = (ntqs_client*)llq_pop(ntqs->queues[i]);
			sockclose(client->socket);
			free(client);
		}
		llq_delete(&ntqs->queues[i]);
		ret = pthread_mutex_destroy(ntqs->m_queues[i]);
	}
	free((void*)m_queues_array);
	free((void*)ntqs->queues);
	sockclose(ntqs->socket);
	free((void*)ntqs->m_queues);
	llq_delete(&ntqs->q_clients);
	ret = pthread_mutex_destroy(ntqs->m_clients);

	free((void*)ntqs);

	return 0;
}

