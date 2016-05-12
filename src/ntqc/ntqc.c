#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <utl.h>
#include <bs.h>
#include "ntqc.h"
#include "ntq_protocol.h"

#ifndef min
#define min(a, b) ((b > a) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((b > a) ? (b) : (a))
#endif



typedef struct ntq_server {
	int socket;
	char* server_address;
	int server_port;
} ntq_server;


int ntq_connect(ntq_server** pserver, char* server_address, int server_port) {
	int ret = 0;
	ntq_server* server = NULL;

	server = (ntq_server*)malloc(sizeof(ntq_server));
	if (server == NULL) {
		fprintf(stderr, "ntq_connect() error not enough memory\n");
		return -1;
	}

	server->server_address = server_address;
	server->server_port = server_port;

	ret = socktcp(&server->socket);
	if (ret != 0) {
		fprintf(stderr, "ntq_connect() error socket creation failed\n");
		return -1;
	}
	ret = sockconnect(server->socket, server->server_address, server->server_port);
	if (ret != 0) {
		fprintf(stderr, "ntq_connect() error socket connect failed\n");
		sockclose(server->socket);
		return -1;
	}

	*pserver = server;
	return 0;
}

int ntq_disconnect(ntq_server** pserver) {
	ntq_server* server = NULL;

	if (pserver == NULL) {
		return 0;
	}
	server = *pserver;


	sockclose(server->socket);

	free(server);
	server = NULL;
	*pserver = server;
	return 0;
}



/**
 * returns the total number of bytes pushed
 */
int ntq_push(ntq_server* server, int queue_id, const void* task_data, const int size) {
	int ret = 0;
	char* buf;
	int bufsize;
	ntq_req_header* req_header;
	void* req_data;

	bufsize = sizeof(ntq_req_header) + size;
	buf = (char*)malloc(bufsize);
	if (buf == NULL) {
		fprintf(stderr, "ntq_push() error not enough memory\n");
		return -1;
	}

	req_header = (ntq_req_header*)&buf[0];
	req_header->req_type = NTQ_PUSH;
	req_header->queue_id = queue_id;
	req_header->size = size;
	req_data = (void*)&buf[sizeof(ntq_req_header)];
	memcpy(req_data, task_data, size);

	ret = sendl(server->socket, (const void*)buf, bufsize, 0);
	if (ret < 0) {
		fprintf(stderr, "ntq_push() error on socket send\n");
		free((void*)buf);
		return -1;
	}
	else if (ret < bufsize) {
		fprintf(stderr, "ntq_push() error did not send all data\n");
		free((void*)buf);
		ret -= sizeof(ntq_req_header);
		return ret;
	}

	ret -= sizeof(ntq_req_header);
	free((void*)buf);

	return ret;
}


/*
 * Returns the total number of bytes popped
 * when input *psize is the max size of task_data
 * at output *psize is the size written to task_data
 */
int ntq_pop(ntq_server* server, int queue_id, void* task_data, int *psize) {
	int ret = 0;
	ntq_req_header req_header_data;
	ntq_req_header* req_header;
	ntqs_q_element_hdr qelem_header_data;
	ntqs_q_element_hdr* qelem_header;
	int size;
	void* data;

	req_header = &req_header_data;
	req_header->req_type = NTQ_POP;
	req_header->queue_id = queue_id;
	req_header->size = 0;

	ret = sendl(server->socket, (const void*)req_header, sizeof(ntq_req_header), 0);
	if (ret < 0) {
		fprintf(stderr, "ntq_pop() error on socket send\n");
		return -1;
	}
	else if (ret < sizeof(ntq_req_header)) {
		fprintf(stderr, "ntq_pop() error did not send all data\n");
		return -1;
	}

	qelem_header = &qelem_header_data;
	ret = recvl(server->socket, (void*)qelem_header, sizeof(ntqs_q_element_hdr), 0);
	if (ret < 0) {
		fprintf(stderr, "ntq_pop() error on socket recv\n");
		return -1;
	}
	else if (ret < sizeof(ntqs_q_element_hdr)) {
		fprintf(stderr, "ntq_pop() error did not recv all data\n");
		return -1;
	}

	size = qelem_header->size;

	if (size == 0) {
		// queue empty
		return 0;
	}
	else {
		// queue not empty
		data = malloc(size);
		if (data == NULL) {
			fprintf(stderr, "ntq_pop() error not enough memory\n");
			return -1;
		}

		ret = recvl(server->socket, data, size, 0);
		if (ret < 0) {
			fprintf(stderr, "ntq_pop() error on socket recv\n");
			return -1;
		}
		else if (ret < size) {
			fprintf(stderr, "ntq_pop() error did not recv all data\n");
		}
		size = ret;

		*psize = min(*psize, size);
		memcpy(task_data, data, *psize);

		free(data);

		return ret;
	}
}


