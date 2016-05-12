#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "utl.h"
#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <unistd.h>
#endif



typedef struct llq_node {
	void* data;
	struct llq_node* next;
} llq_node;

typedef struct llq {
	llq_node* first;
	llq_node* last;
	int len;
} llq;

int llq_create(llq** pqueue) {
	llq* queue = NULL;

	queue = (llq*)malloc(sizeof(llq));
	if (queue == NULL) {
		*pqueue = NULL;
		return -1;
	}

	queue->first = NULL;
	queue->last = NULL;
	queue->len = 0;

	*pqueue = queue;
	return 0;
}

void llq_delete(llq** pqueue) {
	llq* queue = *pqueue;
	if (queue == NULL) {
		return;
	}
	free((void*)queue);
	queue = NULL;
	*pqueue = queue;
	return;
}

int llq_isempty(llq* queue) {
	return (queue->first == NULL && queue->last == NULL);
	//return (queue->first == NULL); // equivalent
	//return (queue->last == NULL);  // equivalent
	//return (queue->len == 0); // equivalent
}

int llq_length(llq* queue) {
	return queue->len;
}

int llq_push(llq* queue, void* data) {
	llq_node* new_node = NULL;

	new_node = (llq_node*)malloc(sizeof(llq_node));
	if (new_node == NULL) {
		return -1;
	}
	new_node->data = data;
	new_node->next = NULL;

	if (llq_isempty(queue)) {
		queue->first = new_node;
		queue->last = new_node;
		queue->len++;
		return 0;
	} else {
		queue->last->next = new_node;
		queue->last = new_node;
		queue->len++;
		return 0;
	}
}

void* llq_pop(llq* queue) {
	llq_node* first_node = NULL;
	void* data;

	first_node = queue->first;
	if (llq_isempty(queue)) {
		return NULL;
	}

	if (queue->first == queue->last) {
		queue->first = NULL;
		queue->last = NULL;
		queue->len--;
	}
	else {
		queue->first = queue->first->next;
		queue->len--;
	}

	data = first_node->data;
	free(first_node);

	return data;
}

#ifdef WIN32

int rand_r (unsigned int *seed)
{
	unsigned int result;
	unsigned int next = *seed;

	next = next * 1103515245 + 12345;
	result = (next >> 16) & RAND_MAX; /** result = (next / 65536) % 32768; **/

	*seed = next;
	return (int)result;
}

#endif





double drawnormal_r(unsigned int *prseed)
{
	double U1, U2, drawn, pi;

	pi = 3.141592653589793;

	U1 = (rand_r(prseed)+1)/((double)((unsigned int)RAND_MAX+1)); /** we don't want a value of 0 otherwise drawn = +oo **/
	U2 = (rand_r(prseed)+1)/((double)((unsigned int)RAND_MAX+1));

	drawn = sqrt(-2*log(U1))*cos(2*pi*U2);

	return drawn;
}


void UTLsleep(int ms)
{
#ifdef WIN32
	Sleep(ms);
#else
	usleep(1000*ms);
#endif
}


/**
 * Safe freeing
 * free an address and set it to NULL to prevent double freeing
 */
void UTLFree(void **paddress)
{
	void *address = *paddress;

	if (address == NULL) goto BACK;

	/**printf("freeing array at %p\n", address);**/
	free(address);
	address = NULL; /** prevents double freeing **/

	BACK:
	*paddress = address;
}


/** print vector **/
void UTLShowVector(int n, double *vector)
{
	int j;

	for (j = 0; j < n; j++){
		printf(" %g", vector[j]);
	}
	printf("\n");
}

/** print vector **/
void UTLShowIntVector(int n, int *vector)
{
	int j;

	for (j = 0; j < n; j++){
		printf(" %d", vector[j]);
	}
	printf("\n");
}


char* UTLGetTimeStamp(void)
{
	time_t timestamp;

	timestamp = time(0);
	return (char *) ctime(&timestamp);
}


int UTLticks_ms()
{
#ifdef WIN32
	return (int)GetTickCount();
#else
	struct tms tm;
	return (int)(times(&tm) * 10);
#endif
}


