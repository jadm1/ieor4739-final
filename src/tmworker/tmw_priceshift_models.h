#ifndef TMW_PRICESHIFT_MODELS_H
#define TMW_PRICESHIFT_MODELS_H

#ifdef __cplusplus
extern "C" {
#endif


// Price shift Models

typedef struct trade_impact_priceshift_model {
	void (*compute_priceshifts)(double* d, int N, void* params);
	void* params;
} trade_impact_priceshift_model;


typedef struct priceshift_alphapi_prm {
	double alpha;
	double pi;
} priceshift_alphapi_prm;
void compute_priceshifts_alphapi(double* d, int N, void* params);

typedef struct priceshift_alphalog_prm {
	double alpha;
} priceshift_alphalog_prm;
void compute_priceshifts_alphalog(double* d, int N, void* params);

typedef struct priceshift_sqrtinv_prm {
} priceshift_sqrtinv_prm;
void compute_priceshifts_sqrtinv(double* d, int N, void* params);





#ifdef __cplusplus
}
#endif

#endif

