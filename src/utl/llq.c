
/**
 *   This file implements a simple queue data structure based on linked lists
 *   to hold any type of data (void*)
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "utl.h"

/**
 * Linked list node (one element of the queue)
 */
typedef struct llq_node {
	void* data;
	struct llq_node* next;
} llq_node;

/**
 * Queue struct based on the first and last node
 * First and last node are needed to perform FIFO operations
 */
typedef struct llq {
	llq_node* first;
	llq_node* last;
	int len;
} llq;


/**
 * Allocate llq struct
 */
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

/**
 * Delete llq struct
 */
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

/**
 * Returns 1 if the queue has no element, 0 otherwise
 */
int llq_isempty(llq* queue) {
	return (queue->first == NULL && queue->last == NULL);
	//return (queue->first == NULL); // equivalent
	//return (queue->last == NULL);  // equivalent
	//return (queue->len == 0); // equivalent
}

/**
 * Returns the number of elements in the queue (num of elements in the linked list)
 */
int llq_length(llq* queue) {
	return queue->len;
}

/**
 * Push a new element to the queue (add at the end)
 */
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

/**
 * Pop a new element from the queue (remove from the beginning)
 * Returns null if the queue is empty
 */
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

