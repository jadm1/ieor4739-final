#ifndef TMW_PROB_MODELS_H
#define TMW_PROB_MODELS_H

#ifdef __cplusplus
extern "C" {
#endif

// Probability Models


typedef struct trade_impact_prob_model {
	void (*compute_probs)(double* probs, int N, void* params);
	void* params;
} trade_impact_prob_model;

typedef struct prob_squareinv_prm {
} prob_squareinv_prm;
void compute_probs_squareinv(double* probs, int N, void* params);
typedef struct prob_deterministic_prm {
} prob_deterministic_prm;
void compute_probs_deterministic(double* probs, int N, void* params);


#ifdef __cplusplus
}
#endif

#endif

