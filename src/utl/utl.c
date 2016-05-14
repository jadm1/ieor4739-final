
/**
 * This file contains utility functions for miscellaneous purposes
 */


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



#ifdef WIN32
/**
 * Generate a discrete uniform random int based on the seed, and update that seed
 * This function is thread safe provided the seed is a non-shared variable
 */
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


/**
 * Generate a normal rv based on the rand_r seed, and update that seed
 * This function is thread safe provided the seed is a non-shared variable
 */
double drawnormal_r(unsigned int *prseed)
{
	double U1, U2, drawn, pi;

	pi = 3.141592653589793;

	U1 = (rand_r(prseed)+1)/((double)((unsigned int)RAND_MAX+1)); /** we don't want a value of 0 otherwise drawn = +oo **/
	U2 = (rand_r(prseed)+1)/((double)((unsigned int)RAND_MAX+1));

	drawn = sqrt(-2*log(U1))*cos(2*pi*U2);

	return drawn;
}

/**
 * Sleeps the specified number of milliseconds
 */
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


/** print vector of doubles **/
void UTLShowVector(int n, double *vector)
{
	int j;

	for (j = 0; j < n; j++){
		printf(" %g", vector[j]);
	}
	printf("\n");
}

/** print vector of integers **/
void UTLShowIntVector(int n, int *vector)
{
	int j;

	for (j = 0; j < n; j++){
		printf(" %d", vector[j]);
	}
	printf("\n");
}

/**
 * Returns a timestamp
 */
char* UTLGetTimeStamp(void)
{
	time_t timestamp;

	timestamp = time(0);
	return (char *) ctime(&timestamp);
}

/**
 * Returns a number that is incremented every ms on average.
 */
int UTLticks_ms()
{
#ifdef WIN32
	return (int)GetTickCount();
#else
	struct tms tm;
	return (int)(times(&tm) * 10);
#endif
}


