#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <utl.h>
#include <bs.h>
#include <ntqc.h>

typedef struct test_client {
	ntq_server* server;
	char* server_address;
	int server_port;
} test_client;

#define INPUT_SIZE 100

int testmain(test_client* client, int push, int qid, int req_num) {
	int ret = 0;
	char input[INPUT_SIZE];
	int size;
	int iter;

	ret = loadsocklib();
	if (ret < 0) {
		fprintf(stderr, "clientmain(): error cannot start sockets library\n");
		return -1;
	}

	printf("Connecting to %s:%d ...\n", client->server_address, client->server_port);
	fflush(stdout);

	ret = ntq_connect(&client->server, client->server_address, client->server_port);
	if (ret != 0) {
		fprintf(stderr, "clientmain(): error could not connect to ntq server\n");
		return -1;
	}

	printf("Connected\n");

	if (push) {
		printf("push data (<%d B/message). say exit to quit\n", INPUT_SIZE);
		iter = 0;
		while (1) {
			printf(">");
			scanf("%s", input);
			if (!strcmp(input, "exit")) {
				break;
			}
			ret = ntq_push(client->server, qid, (const void*)input, strlen(input)+1);
			if (ret < 0) {
				fprintf(stderr, "clientmain(): error ntq_push() failed\n");
				break;
			}
			printf("pushed %d bytes\n", ret);
			iter++;
			if (req_num > 0 && iter >= req_num) {
				break;
			}
		}
	}
	else {
		printf("pop data\n");

		iter = 0;
		while (1) {
			size = INPUT_SIZE-1;
			ret = ntq_pop(client->server, qid, (void*)input, &size);
			if (ret < 0) {
				fprintf(stderr, "clientmain(): error ntq_push() failed\n");
				break;
			}
			else if (ret == 0) {
				printf("queue empty\n");
				break;
			}
			else {
				printf(">%s\n", input);
				printf("popped %d bytes (saved %d bytes)\n", ret, size);
			}
			iter++;
			if (req_num > 0 && iter >= req_num) {
				break;
			}
		}

	}

	ret = ntq_disconnect(&client->server);
	if (ret != 0) {
		return -1;
	}

	ret = freesocklib();
	if (ret < 0) {
		return -1;
	}

	return ret;
}


int main(int argc, char** argv) {
	int ret = 0;
	int i;

	int push;

	test_client* client;

	char *address;
	int port;
	int qid;
	int req_num;
	//int verbose;


	// Default parameters
	address = "localhost";
	port = 12345;
	push = 0;
	qid = 0;
	req_num = 0; // 0 means unlimited
	//verbose = 0;

	if (argc <= 0){
		printf("Usage: %s [-a address] [-p port] [-o/-i push/pop] [-q queue id] [-n max num of requests]\n", argv[0]);
		return -1;
	}
	for (i = 1; i < argc; i++){
		if (!strcmp(argv[i], "-a")) {
			i += 1;
			address = argv[i];
		}
		else if (!strcmp(argv[i], "-p")) {
			i += 1;
			port = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-o")) {
			push = 1;
		}
		else if (!strcmp(argv[i], "-i")) {
			push = 0;
		}
		else if (!strcmp(argv[i], "-q")) {
			i += 1;
			qid = atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-n")) {
			i += 1;
			req_num = atoi(argv[i]);
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
	client->server_address = address;
	client->server_port = port;


	ret = testmain(client, push, qid, req_num);
	if (ret < 0) {
		fprintf(stderr, "main(): error in client()\n");
		free(client);
		return -1;
	}

	free(client);



	return ret;
}

