
#include "tm_priceshift_models.h"

#include <math.h>


/**
 * price shift models
 */

void compute_priceshifts_alphapi(double* d, int N, void* params) {
	priceshift_alphapi_prm* par = (priceshift_alphapi_prm*)params;
	double alpha = par->alpha;
	double pi = par->pi;
	int n;

	for (n = 0; n <= N; n++) {
		d[n] = 1.0 - alpha*pow((double)n, pi);
	}

	return;
}


void compute_priceshifts_alphalog(double* d, int N, void* params) {
	priceshift_alphalog_prm* par = (priceshift_alphalog_prm*)params;
	double alpha = par->alpha;
	int n;

	for (n = 0; n <= N; n++) {
		d[n] = 1.0 - alpha*log(1.0 + (double)n);
	}

	return;
}

void compute_priceshifts_sqrtinv(double* d, int N, void* params) {
	//priceshifts_sqrtinv_params* par = (priceshifts_sqrtinv_params*)params;
	int n;

	for (n = 0; n <= N; n++) {
		d[n] = 1.0/sqrt(1.0 + (double)n);
	}

	return;
}

