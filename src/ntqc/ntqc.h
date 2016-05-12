#ifndef NTQC_H
#define NTQC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ntq_server ntq_server;

int ntq_connect(ntq_server** pserver, char* server_address, int server_port);
int ntq_disconnect(ntq_server** pserver);
int ntq_push(ntq_server* server, int queue_id, const void* task_data, const int size);
int ntq_pop(ntq_server* server, int queue_id, void* task_data, int *psize);


#ifdef __cplusplus
}
#endif

#endif

