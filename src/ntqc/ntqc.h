#ifndef NTQC_H
#define NTQC_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  This is the ntq client header file and typically the only file that
 *  needs to be included by an external project who wishes to communicate
 *  with the ntq server
 */



typedef struct ntq_server ntq_server;

/**
 * Connect to a remote ntq server and initialize the data structure associated with it
 */
int ntq_connect(ntq_server** pserver, char* server_address, int server_port);
/**
 * Disconnect from a remote ntq server and frees the data structure associated with it
 */
int ntq_disconnect(ntq_server** pserver);
/**
 * Push (upload) data of a given size to a given queue
 */
int ntq_push(ntq_server* server, int queue_id, const void* task_data, const int size);
/**
 *  Pops one element from a given queue and downloads its data
 *  at input *psize contains the max size of the task_data buffer in bytes
 *  at the output *psize contains the size that was written to the task_data buffer
 *  and the function returns the number of bytes that were actually received
 */
int ntq_pop(ntq_server* server, int queue_id, void* task_data, int *psize);


#ifdef __cplusplus
}
#endif

#endif

