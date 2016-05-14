#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <utl.h>
#include <bs.h>
#include <ntqc.h>
#include <tm_priceshift_models.h>
#include <tm_prob_models.h>

#include <tm_protocol.h>


/*
 *  This is the job supplier program which reads an input file,
 *  and sends its data to the (first) job input queue of a ntq server
 */

typedef struct test_client {
	ntq_server* server;
	char* server_address;
	int server_port;
} test_client;



int jobsuppliermain(test_client* client, char* address, const int port, char* input_filename) {
	int ret = 0;

	FILE* input_file;
	char b[100];
	int nb_problems;
	int i;
	int job_id;
	int N, T;
	char* buf, * ptr;
	int bufsize;

	priceshift_alphalog_prm priceshift_alphalog_params;
	priceshift_alphapi_prm priceshift_alphapi_params;
	//prob_squareinv_prm prob_squareinv_params;
	//prob_deterministic_prm prob_deterministic_params;

	int priceshift_model_choice;
	int prob_model_choice;

	client->server_address = address;
	client->server_port = port;


	ret = loadsocklib();
	if (ret < 0) {
		fprintf(stderr, "jobsuppliermain(): error cannot start sockets library\n");
		return -1;
	}

	printf("Connecting to %s:%d ...\n", client->server_address, client->server_port);
	fflush(stdout);

	ret = ntq_connect(&client->server, client->server_address, client->server_port);
	if (ret != 0) {
		fprintf(stderr, "jobsuppliermain(): error could not connect to ntq server\n");
		return -1;
	}

	printf("Connected\n");

	input_file = fopen(input_filename, "r");

	fscanf(input_file, "%s", b);
	nb_problems = atoi(b);

	printf("nb_problems: %d\n", nb_problems);
	for (i = 0; i < nb_problems; i++) {
		bufsize = 0;

		// parsing input data from file
		fscanf(input_file, "%s", b);
		job_id = atoi(b);
		bufsize += sizeof(int);
		fscanf(input_file, "%s", b);
		N = atoi(b);
		bufsize += sizeof(int);
		fscanf(input_file, "%s", b);
		T = atoi(b);
		bufsize += sizeof(int);


		bufsize += sizeof(int);
		fscanf(input_file, "%s", b);
		if (!strcmp(b, "alphapi")) {
			fscanf(input_file, "%s", b);
			priceshift_alphapi_params.alpha = atof(b);
			bufsize += sizeof(double);
			fscanf(input_file, "%s", b);
			priceshift_alphapi_params.pi = atof(b);
			bufsize += sizeof(double);
			priceshift_model_choice = PSM_ALPHAPI;
		}
		else if (!strcmp(b, "alphalog")) {
			fscanf(input_file, "%s", b);
			priceshift_alphalog_params.alpha = atof(b);
			bufsize += sizeof(double);
			priceshift_model_choice = PSM_ALPHALOG;
		}
		else {
			fprintf(stderr, "input file parse error %s\n", b);
			return -1;
		}

		bufsize += sizeof(int);
		fscanf(input_file, "%s", b);
		if (!strcmp(b, "squareinv")) {
			prob_model_choice = PRM_SQUAREINV;
		}
		else if (!strcmp(b, "deterministic")) {
			prob_model_choice = PRM_DETERMINISTIC;
		}
		else {
			fprintf(stderr, "input file parse error %s\n", b);
			return -1;
		}

		buf = (char*)calloc(bufsize, sizeof(char));
		if (buf == NULL) {
			fprintf(stderr, "error: not enough memory\n");
			return -1;
		}


		// preparing data to send
		ptr = buf;

		*(int*)ptr = htonl(job_id);
		ptr += sizeof(int);
		*(int*)ptr = htonl(N);
		ptr += sizeof(int);
		*(int*)ptr = htonl(T);
		ptr += sizeof(int);
		*(int*)ptr = htonl(priceshift_model_choice);
		ptr += sizeof(int);
		switch (priceshift_model_choice) {
		case PSM_ALPHAPI:
			*(double*)ptr = priceshift_alphapi_params.alpha;
			ptr += sizeof(double);
			*(double*)ptr = priceshift_alphapi_params.pi;
			ptr += sizeof(double);
			break;
		case PSM_ALPHALOG:
			*(double*)ptr = priceshift_alphalog_params.alpha;
			ptr += sizeof(double);
			break;
		default:
			return -1;
		}
		*(int*)ptr = prob_model_choice;
		ptr += sizeof(int);
		switch (prob_model_choice) {
		case PRM_SQUAREINV:
			break;
		case PRM_DETERMINISTIC:
			break;
		default:
			return -1;
		}

		// send data
		ret = ntq_push(client->server, QUEUE_JOBINPUTS, (const void*)buf, bufsize);
		if (ret < 0) {
			fprintf(stderr, "jobsuppliermain(): error could not push data\n");
			return -1;
		}

		free((void*)buf);
	}

	fclose(input_file);

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
	int parse_error;

	test_client* client;

	char* input_filename;
	char* address;
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
		printf("Usage: %s <input file> [-a address] [-p port]\n", argv[0]);
		return -1;
	}



	client = (test_client*)malloc(sizeof(test_client));
	if (client == NULL) {
		fprintf(stderr, "main(): error not enough memory\n");
		return -1;
	}

	ret = jobsuppliermain(client, address, port, input_filename);
	if (ret < 0) {
		fprintf(stderr, "main(): error in jobsuppliermain()\n");
		free(client);
		return -1;
	}

	free(client);



	return ret;
}

