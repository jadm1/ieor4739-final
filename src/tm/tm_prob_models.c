
#include "tm_prob_models.h"

#include <math.h>


/**
 * probability models
 */

/*
 * probs is a (N+1)x(N+1) matrix representing prob(k'|k)
 * 0<=k<=N
 * 0<=kp<=k
 */
#define prob(kp, k) probs[(k)*(N+1) + (kp)]


void compute_probs_squareinv(double* probs, int N, void* params) {
	//priceshifts_default_params* par = (priceshifts_default_params*)params;
	int k;
	int kp; // (k prime)
	double a;
	double s;

	// assign weights
	for (k = 0; k <= N; k++) {
		for (kp = 0; kp <= k; kp++) {
			a = (double)(k - kp + 1);
			a = 1.0/(a*a);
			prob(kp, k) = a;
		}
	}

	// normalize
	for (k = 0; k <= N; k++) {
		s = 0.0;
		for (kp = 0; kp <= k; kp++)
			s += prob(kp, k);
		for (kp = 0; kp <= k; kp++) {
			prob(kp, k) /= s;
		}
	}

	return;
}

void compute_probs_deterministic(double* probs, int N, void* params) {
	//priceshifts_deterministic_params* par = (priceshifts_deterministic_params*)params;
	int k;
	int kp; // (k prime)
	double a;
	double s;

	// assign weights

	for (k = 0; k <= N; k++) {
		for (kp = 0; kp < k; kp++) {
			prob(kp, k) = 0.0;
		}
		prob(k, k) = 1.0;
	}

	// no need to normalize

	return;
}

