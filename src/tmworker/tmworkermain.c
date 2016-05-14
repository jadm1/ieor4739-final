#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <utl.h>
#include <bs.h>
#include <ntqc.h>

#include <tmw_opt.h>
#include <tmw_priceshift_models.h>
#include <tmw_prob_models.h>
#include <tmw_protocol.h>


typedef struct test_client {
	ntq_server* server;
	char* server_address;
	int server_port;
} test_client;



int resultsavermain(test_client* client, char* address, const int port, int verbose) {
	int ret = 0;

	trade_impact_problem* pb;
	trade_impact_priceshift_model priceshift_model;
	trade_impact_prob_model prob_model;
	priceshift_alphalog_prm priceshift_alphalog_params;
	priceshift_alphapi_prm priceshift_alphapi_params;
	prob_squareinv_prm prob_squareinv_params;
	prob_deterministic_prm prob_deterministic_params;

	int t;
	int iter;
	int bufsize;
	char* buf, * ptr;

	client->server_address = address;
	client->server_port = port;

	int job_id;
	int N;
	int T;
	int priceshift_model_choice;
	int prob_model_choice;


	//double profit;
	int* optimal_sells;
	double* optimal_prices;


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



	iter = 0;
	while (1) {
		printf("-----------\n");
		printf("Getting job\n");

		// Receive job info from input queue

		bufsize = 1000;
		buf = (char*)calloc(bufsize, sizeof(char));
		if (buf == NULL) {
			fprintf(stderr, "workermain(): error not enough memory\n");
			return -1;
		}

		ret = ntq_pop(client->server, QUEUE_JOBINPUTS, (void*)buf, &bufsize);
		if (ret < 0) {
			free((void*)buf);
			fprintf(stderr, "workermain(): error ntq_pop() failed\n");
			break;
		}
		else if (ret == 0) {
			free((void*)buf);
			printf("Job queue is empty\n");
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

			priceshift_model_choice = ntohl(*(int*)ptr);
			ptr += sizeof(int);
			switch (priceshift_model_choice) {
			case PSM_ALPHAPI:
				priceshift_alphapi_params.alpha = *(double*)ptr;
				ptr += sizeof(double);
				priceshift_alphapi_params.pi = *(double*)ptr;
				ptr += sizeof(double);
				priceshift_model.compute_priceshifts = compute_priceshifts_alphapi;
				priceshift_model.params = (void*)&priceshift_alphapi_params;
				break;
			case PSM_ALPHALOG:
				priceshift_alphalog_params.alpha = *(double*)ptr;
				ptr += sizeof(double);
				priceshift_model.compute_priceshifts = compute_priceshifts_alphalog;
				priceshift_model.params = (void*)&priceshift_alphalog_params;
				break;
			default:
				return -1;
			}

			prob_model_choice = ntohl(*(int*)ptr);
			ptr += sizeof(int);
			switch (prob_model_choice) {
			case PRM_SQUAREINV:
				prob_model.compute_probs = compute_probs_squareinv;
				prob_model.params = (void*)&prob_squareinv_params;
				break;
			case PRM_DETERMINISTIC:
				prob_model.compute_probs = compute_probs_deterministic;
				prob_model.params = (void*)&prob_deterministic_params;
				break;
			default:
				return -1;
			}

			free((void*)buf);

			// Work

			if (verbose >= 1) {
				printf("Working on job %d\n", job_id);
			}

			ret = trade_imp_pb_create(&pb, N, T, &priceshift_model, &prob_model);
			if (ret != 0) {
				fprintf(stderr, "workermain(): error in trade_impact_problem_create()\n");
				return -1;
			}
			ret = trade_imp_pb_optimize(pb, verbose);
			if (ret < 0) {
				fprintf(stderr, "workermain(): error in optimize_trade()\n");
				return -1;
			}
			printf("Optimal value for trade sequencing = %g\n", F(pb, 0, pb->N));


			optimal_sells = (int*)calloc(T, sizeof(int));
			optimal_prices = (double*)calloc(T, sizeof(double));
			if (optimal_sells == NULL || optimal_prices == NULL) {
				fprintf(stderr, "workermain(): error not enough memory\n");
				return -1;
			}

			trade_imp_pb_fw_prop_deterministic(pb, verbose, optimal_sells, optimal_prices);

			ret = trade_imp_pb_delete(&pb);


			// Submit results to output queue

			bufsize = 0;
			// determine bufsize
			bufsize += sizeof(int);
			bufsize += sizeof(int);
			bufsize += sizeof(int);
			bufsize += T*sizeof(int);
			bufsize += T*sizeof(double);

			// allocate data and prepare buf
			buf = (char*)calloc(bufsize, sizeof(char));
			if (buf == NULL) {
				fprintf(stderr, "workermain(): error not enough memory\n");
				return -1;
			}

			ptr = buf;
			*(int*)ptr = htonl(job_id);
			ptr += sizeof(int);
			*(int*)ptr = htonl(N);
			ptr += sizeof(int);
			*(int*)ptr = htonl(T);
			ptr += sizeof(int);
			for (t = 0; t <= T-1; t++) {
				*(int*)ptr = htonl(optimal_sells[t]);
				ptr += sizeof(int);
			}
			for (t = 0; t <= T-1; t++) {
				*(double*)ptr = (optimal_prices[t]);
				ptr += sizeof(double);
			}

			// send data

			ret = ntq_push(client->server, QUEUE_JOBRESULTS, (const void*)buf, bufsize);
			if (ret < 0) {
				fprintf(stderr, "workermain(): error could not push data\n");
				return -1;
			}

			free((void*)optimal_prices);
			free((void*)optimal_sells);
			free((void*)buf);


			printf("Job done\n");
		}
		iter++;
	}
	printf("-----------\n");




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

	char *address;
	int port;
	int verbose;


	// Default parameters
	address = "localhost";
	port = 12345;
	verbose = 0;

	if (argc <= 0){
		printf("Usage: %s [-a address] [-p port]\n", argv[0]);
		return -1;
	}
	for (i = 1; i < argc; i++){
		if (!strcmp(argv[i], "-v")) {
			verbose = 1;
		}
		else if (!strcmp(argv[i], "-vv")) {
			verbose = 2;
		}
		else if (!strcmp(argv[i], "-a")) {
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



	client = (test_client*)malloc(sizeof(test_client));
	if (client == NULL) {
		fprintf(stderr, "main(): error not enough memory\n");
		return -1;
	}

	ret = resultsavermain(client, address, port, verbose);
	if (ret < 0) {
		fprintf(stderr, "main(): error in jobsuppliermain()\n");
		free(client);
		return -1;
	}

	free(client);



	return ret;
}

