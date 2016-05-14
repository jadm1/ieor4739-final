#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ntqs.h"
#include "ntq_protocol.h"


/**
 *  This is a manager thread which runs a loop over all connected clients.
 *  It gets a client from the client queue,
 *  checks if it sent a request
 *  If the client is inactive, it skips the client
 *  Otherwise it will respond to what the client is requesting. (push/pop)
 *  Once it is done the client is pushed to the queue and the next client is processed
 *  If the client behaves badly, does not respect the protocol or does not respond, it is kicked off the server and deleted from the queue
 */

void* ntqmanagerwrapper(void* params) {
	ntqs_manager* manager = (ntqs_manager*)params;
	ntqmanager(manager);
	return NULL;
}
int ntqmanager(ntqs_manager* manager) {
	int ret = 0;
	ntqs_server* ntqs = manager->ntqs;
	ntqs_client* client = NULL;
	ntq_req_header req_header_data;
	int qid;
	char* buf;
	int bufsize;
	ntqs_q_element_hdr qelem_header_data;
	ntqs_q_element_hdr* qelem_header;
	void* data;

	int num_clients;
	int iter;
	int all_clients_idle;
	int go_sleep;
	int kick_client;
	int got_interrupt;

	pthread_mutex_lock(ntqs->m_console);
	printf("M%d: thread started ...\n", manager->id);
	pthread_mutex_unlock(ntqs->m_console);

	got_interrupt = 0;


	iter = 0;
	all_clients_idle = 1;
	while (1) {
		iter++;

		// get client from queue
		pthread_mutex_lock(ntqs->m_clients);
		num_clients = llq_length(ntqs->q_clients);
		client = llq_pop(ntqs->q_clients);
		pthread_mutex_unlock(ntqs->m_clients);

		if (client != NULL) { // equivalent to if (num_clients > 0)

			kick_client = 0; // boolean to kick client in case of bad behavior

			// check if the socket received data from the currently processed client
			ret = sockreadable(client->socket);
			if (ret < 0) {
				// socket error
				pthread_mutex_lock(ntqs->m_console);
				fprintf(stderr, "M%d: ntqmanager() %s:%d  error socket select failed\n", manager->id, client->remote_ip, client->remote_port); fflush(stderr);
				pthread_mutex_unlock(ntqs->m_console);
				kick_client = 1;
			}

			if (ret == 0) {
				// skip client (the client is not talking)
			}
			else if (ret > 0) {
				// client talking
				all_clients_idle = 0;
				//pthread_mutex_lock(ntqs->m_console);
				//printf("M%d: %s:%d is talking\n", manager->id, client->remote_ip, client->remote_port);
				//pthread_mutex_unlock(ntqs->m_console);

				ret = recvl(client->socket, (void*)&req_header_data, sizeof(ntq_req_header), 0);
				if (ret < 0) {
					pthread_mutex_lock(ntqs->m_console);
					fprintf(stderr, "M%d: ntqmanager() %s:%d  error socket recv failed\n", manager->id, client->remote_ip, client->remote_port); fflush(stderr);
					pthread_mutex_unlock(ntqs->m_console);
					kick_client = 1;
				}
				else if (ret == 0) {
					kick_client = 1;
				}
				else if (ret < sizeof(ntq_req_header)) {
					pthread_mutex_lock(ntqs->m_console);
					fprintf(stderr, "M%d: ntqmanager() %s:%d  error socket recv did not get all bytes\n", manager->id, client->remote_ip, client->remote_port); fflush(stderr);
					pthread_mutex_unlock(ntqs->m_console);
					kick_client = 1;
				}
				else {


					switch (req_header_data.req_type) {
					case NTQ_PUSH: {
						pthread_mutex_lock(ntqs->m_console);
						printf("M%d: %s:%d -> queue %d push %d bytes\n", manager->id, client->remote_ip, client->remote_port, req_header_data.queue_id, req_header_data.size);
						pthread_mutex_unlock(ntqs->m_console);


						qid = req_header_data.queue_id;
						if (qid < 0 || qid >= ntqs->n_queues) {
							// illegal number of queues
							pthread_mutex_lock(ntqs->m_console);
							fprintf(stderr, "M%d: %s:%d error bad queue id\n", manager->id, client->remote_ip, client->remote_port); fflush(stderr);
							pthread_mutex_unlock(ntqs->m_console);
							kick_client = 1;
							break;
						}

						bufsize = sizeof(ntqs_q_element_hdr) + req_header_data.size;
						buf = (char*)malloc(bufsize);
						if (buf == NULL) {
							pthread_mutex_lock(ntqs->m_console);
							fprintf(stderr, "M%d: ntqmanager() error not enough memory\n", manager->id); fflush(stderr);
							pthread_mutex_unlock(ntqs->m_console);
							return -1;
						}

						qelem_header = (ntqs_q_element_hdr*)&buf[0];
						data = (void*)&buf[sizeof(ntqs_q_element_hdr)];

						qelem_header->size = req_header_data.size;
						ret = recvl(client->socket, data, req_header_data.size, 0);
						if (ret < 0) {
							pthread_mutex_lock(ntqs->m_console);
							fprintf(stderr, "M%d: ntqmanager() %s:%d error socket recv failed\n", manager->id, client->remote_ip, client->remote_port); fflush(stderr);
							pthread_mutex_unlock(ntqs->m_console);
							kick_client = 1;
						}
						else if (ret < req_header_data.size) {
							pthread_mutex_lock(ntqs->m_console);
							fprintf(stderr, "M%d: ntqmanager() %s:%d error socket recv did not get all bytes\n", manager->id, client->remote_ip, client->remote_port); fflush(stderr);
							pthread_mutex_unlock(ntqs->m_console);
							kick_client = 1;
						}
						else {

							pthread_mutex_lock(ntqs->m_queues[qid]);
							llq_push(ntqs->queues[qid], buf);
							pthread_mutex_unlock(ntqs->m_queues[qid]);

							pthread_mutex_lock(ntqs->m_console);
							printf("M%d: %s:%d -> push saved to queue %d\n", manager->id, client->remote_ip, client->remote_port, req_header_data.queue_id);
							pthread_mutex_unlock(ntqs->m_console);

						}
					} break;
					case NTQ_POP: {
						pthread_mutex_lock(ntqs->m_console);
						printf("M%d: %s:%d -> queue %d pop\n", manager->id, client->remote_ip, client->remote_port, req_header_data.queue_id);
						pthread_mutex_unlock(ntqs->m_console);

						qid = req_header_data.queue_id;
						if (qid < 0 || qid >= ntqs->n_queues) {
							// illegal number of queues
							pthread_mutex_lock(ntqs->m_console);
							fprintf(stderr, "M%d: %s:%d error bad queue id\n", manager->id, client->remote_ip, client->remote_port); fflush(stderr);
							pthread_mutex_unlock(ntqs->m_console);
							kick_client = 1;
							break;
						}

						pthread_mutex_lock(ntqs->m_queues[qid]);
						buf = llq_pop(ntqs->queues[qid]);
						pthread_mutex_unlock(ntqs->m_queues[qid]);

						if (buf == NULL) {
							// queue empty
							qelem_header = &qelem_header_data;
							qelem_header->size = 0;
							ret = sendl(client->socket, (const void*)qelem_header, sizeof(ntqs_q_element_hdr), 0);
							if (ret < 0) {
								pthread_mutex_lock(ntqs->m_console);
								fprintf(stderr, "M%d: ntqmanager() %s:%d error socket send failed\n", manager->id, client->remote_ip, client->remote_port); fflush(stderr);
								pthread_mutex_unlock(ntqs->m_console);
								kick_client = 1;
							}
							else if (ret < sizeof(ntqs_q_element_hdr)) {
								pthread_mutex_lock(ntqs->m_console);
								fprintf(stderr, "M%d: ntqmanager() %s:%d error socket send did not send all bytes\n", manager->id, client->remote_ip, client->remote_port); fflush(stderr);
								pthread_mutex_unlock(ntqs->m_console);
								kick_client = 1;
							}
							else {
								// sent pop answer successfuly
								pthread_mutex_lock(ntqs->m_console);
								printf("M%d: %s:%d <- empty queue %d\n", manager->id, client->remote_ip, client->remote_port, req_header_data.queue_id);
								pthread_mutex_unlock(ntqs->m_console);
							}
						}
						else {
							// queue not empty
							qelem_header = (ntqs_q_element_hdr*)&buf[0];
							data = (void*)&buf[sizeof(ntqs_q_element_hdr)];
							bufsize = sizeof(ntqs_q_element_hdr) + qelem_header->size;

							ret = sendl(client->socket, buf, bufsize, 0);
							if (ret < 0) {
								pthread_mutex_lock(ntqs->m_console);
								fprintf(stderr, "M%d: ntqmanager() %s:%d error socket send failed\n", manager->id, client->remote_ip, client->remote_port); fflush(stderr);
								pthread_mutex_unlock(ntqs->m_console);
								kick_client = 1;
							}
							else if (ret < bufsize) {
								pthread_mutex_lock(ntqs->m_console);
								fprintf(stderr, "M%d: ntqmanager() %s:%d error socket send did not send all bytes\n", manager->id, client->remote_ip, client->remote_port); fflush(stderr);
								pthread_mutex_unlock(ntqs->m_console);
								kick_client = 1;
							}
							else {
								// sent pop answer successfuly
								pthread_mutex_lock(ntqs->m_console);
								printf("M%d: %s:%d <- popped %d bytes from queue %d\n", manager->id, client->remote_ip, client->remote_port, qelem_header->size, req_header_data.queue_id);
								pthread_mutex_unlock(ntqs->m_console);
							}

							free(buf);
						}
					} break;
					default: {

					} break;
					}






				}



			}

			if (kick_client) {
				sockclose(client->socket);

				pthread_mutex_lock(ntqs->m_console);
				printf("M%d: %s:%d disconnected\n", manager->id, client->remote_ip, client->remote_port);
				pthread_mutex_unlock(ntqs->m_console);

				free(client);
			}
			else {
				// put the client to the bottom of the client queue
				pthread_mutex_lock(ntqs->m_clients);
				llq_push(ntqs->q_clients, (void*)client);
				pthread_mutex_unlock(ntqs->m_clients);

				//pthread_mutex_lock(ntqs->m_console);
				//printf("M%d: %s:%d sent back to queue\n", manager->id, client->remote_ip, client->remote_port);
				//pthread_mutex_unlock(ntqs->m_console);
			}


		}

		// sleep if there are no clients connected
		// or if none of all connected clients spoke the last time they were checked
		// this is to allow sleeping when the server is idle
		// but prevent too frequent sleeps when there is a lot of activity
		go_sleep = 0;
		if (num_clients == 0) {
			go_sleep = 1;
		}
		else if (iter % num_clients == 0) {
			if (all_clients_idle) {
				go_sleep = 1;
			}
			all_clients_idle = 1;
		}
		if (go_sleep) {
			// flush output
			pthread_mutex_lock(ntqs->m_console);
			fflush(stderr);
			fflush(stdout);
			pthread_mutex_unlock(ntqs->m_console);
			UTLsleep(10);
			// check for interrupts after sleeping (likely to have happened while sleeping)
			pthread_mutex_lock(manager->m_interrupt);
			got_interrupt = manager->interrupt;
			pthread_mutex_unlock(manager->m_interrupt);
			if (got_interrupt) {
				break;
			}
		}

	}


	return 0;
}
