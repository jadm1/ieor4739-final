#ifndef UTL_H
#define UTL_H


/**
 *   This file is the main and only header file for importing utility functions
 *   and llq functions to manage queue data structures
 */


#ifdef __cplusplus
extern "C" {
#endif


typedef struct llq llq;

int llq_create(llq** pqueue);
void llq_delete(llq** pqueue);
int llq_isempty(llq* queue);
int llq_length(llq* queue);
int llq_push(llq* queue, void* data);
void* llq_pop(llq* queue);


#ifdef WIN32
int rand_r (unsigned int *seed);
#endif
double drawnormal_r(unsigned int *prseed);

char *UTLGetTimeStamp(void);
int UTLticks_ms();
void UTLsleep(int ms);
void UTLFree(void **paddress);
void UTLShowVector(int n, double *vector);
void UTLShowIntVector(int n, int *vector);





#ifdef __cplusplus
}
#endif

#endif

