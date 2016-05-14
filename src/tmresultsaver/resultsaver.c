#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <utl.h>
#include <bs.h>
#include <ntqc.h>


#include <tm_protocol.h>

typedef struct test_client {
	ntq_server* server;
	char* server_address;
	int server_port;
} test_client;



int resultsavermain(test_client* client, char* address, const int port, char* output_filename) {
	int ret = 0;

	int t;
	int iter;
	int bufsize;
	char* buf, * ptr;

	client->server_address = address;
	client->server_port = port;

	int job_id;
	int N;
	int T;
	FILE* output_file;


	int* optimal_sells;
	double* optimal_prices;


	ret = loadsocklib();
	if (ret < 0) {
		fprintf(stderr, "resultsavermain(): error cannot start sockets library\n");
		return -1;
	}

	printf("Connecting to %s:%d ...\n", client->server_address, client->server_port);
	fflush(stdout);

	ret = ntq_connect(&client->server, client->server_address, client->server_port);
	if (ret != 0) {
		fprintf(stderr, "resultsavermain(): error could not connect to ntq server\n");
		return -1;
	}

	printf("Connected\n");

	output_file = fopen(output_filename, "a");


	iter = 0;
	while (1) {
		printf("-----------\n");
		printf("Getting job results\n");

		// Receive job info from input queue

		bufsize = 1000000; // should be more than enough
		buf = (char*)calloc(bufsize, sizeof(char));
		if (buf == NULL) {
			fprintf(stderr, "resultsavermain(): error not enough memory\n");
			return -1;
		}

		ret = ntq_pop(client->server, QUEUE_JOBRESULTS, (void*)buf, &bufsize);
		if (ret < 0) {
			free((void*)buf);
			fprintf(stderr, "resultsavermain(): error ntq_pop() failed\n");
			break;
		}
		else if (ret == 0) {
			free((void*)buf);
			printf("Job results queue is empty\n");
			break;
		}
		else {
			printf("Popped %d bytes (saved %d bytes)\n", ret, bufsize);

			// parsing received data
			ptr = buf;
			job_id = ntohl(*(int*)ptr);
			ptr += sizeof(int);
			N = ntohl(*(int*)ptr);
			ptr += sizeof(int);
			T = ntohl(*(int*)ptr);
			ptr += sizeof(int);

			optimal_sells = (int*)calloc(T, sizeof(int));
			optimal_prices = (double*)calloc(T, sizeof(double));
			if (optimal_sells == NULL || optimal_prices == NULL) {
				fprintf(stderr, "resultsavermain(): error not enough memory\n");
				return -1;
			}

			for (t = 0; t <= T-1; t++) {
				optimal_sells[t] = ntohl(*(int*)ptr);
				ptr += sizeof(int);
			}
			for (t = 0; t <= T-1; t++) {
				optimal_prices[t] = *(double*)ptr;
				ptr += sizeof(double);
			}

			// Write results to output file
			fprintf(output_file, "ID:%d N:%d T:%d\n", job_id, N, T);
			fprintf(output_file, "Sell Sequence:");
			for (t = 0; t <= T-1; t++) {
				fprintf(output_file, " %d", optimal_sells[t]);
			}
			fprintf(output_file, "\n");
			fprintf(output_file, "Price Sequence:");
			for (t = 0; t <= T-1; t++) {
				fprintf(output_file, " %g", optimal_prices[t]);
			}
			fprintf(output_file, "\n");

			free((void*)optimal_sells);
			free((void*)optimal_prices);
			free((void*)buf);
			printf("Job results saved\n");
		}
		iter++;
	}
	printf("-----------\n");

	fclose(output_file);

	printf("Exiting...\n");

	ret = ntq_disconnect(&client->server);
	if (ret != 0) {
		return -1;
	}

	ret = freesocklib();
	if (ret < 0) {
		return -1;
	}

	return 0;
}


int main(int argc, char** argv) {
	int ret = 0;
	int i;

	test_client* client;
	int parse_error;


	char* input_filename;
	char *address;
	int port;
	//int verbose;


	// Default parameters
	address = "localhost";
	port = 12345;
	//verbose = 0;

	parse_error = 0;

	if (argc <= 1) {
		parse_error = 1;
	}
	else {
		input_filename = argv[1];

		for (i = 2; i < argc; i++){
			if (!strcmp(argv[i], "-a")) {
				i += 1;
				address = argv[i];
			}
			else if (!strcmp(argv[i], "-p")) {
				i += 1;
				port = atoi(argv[i]);
			}
			else {
				printf("main(): error bad option %s\n", argv[i]);
				return -1;
			}
		}
	}
	if (parse_error) {
		printf("Usage: %s <output file> [-a address] [-p port]\n", argv[0]);
		return -1;
	}


	client = (test_client*)malloc(sizeof(test_client));
	if (client == NULL) {
		fprintf(stderr, "main(): error not enough memory\n");
		return -1;
	}

	ret = resultsavermain(client, address, port, input_filename);
	if (ret < 0) {
		fprintf(stderr, "main(): error in workermain()\n");
		free(client);
		return -1;
	}

	free(client);



	return ret;
}

