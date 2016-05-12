#ifndef UTL_H
#define UTL_H

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


void UTLsleep(int ms);
void UTLFree(void **paddress);
void UTLShowVector(int n, double *vector);
void UTLShowIntVector(int n, int *vector);
char *UTLGetTimeStamp(void);
int UTLticks_ms();



#ifdef __cplusplus
}
#endif

#endif

