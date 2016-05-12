#ifndef NTQ_PROTOCOL_H
#define NTQ_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This file is to be included only in ntqs.c and ntqc.c
 * and is used to represent the format of messages in the server/client communication
 */

// protocol headers and flags
#define NTQ_PUSH 0
#define NTQ_POP  1

typedef struct ntq_req_header {
	int req_type;
	int queue_id;
	int size;
} ntq_req_header;

typedef struct ntqs_q_element_hdr {
	int size;
} ntqs_q_element_hdr;


#ifdef __cplusplus
}
#endif

#endif

