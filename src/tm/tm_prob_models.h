#ifndef TM_PROB_MODELS_H
#define TM_PROB_MODELS_H

#ifdef __cplusplus
extern "C" {
#endif

// Probability Models

/**
 * This structure represents a probability (p(k'|k)) model
 * It has a compute_probs function used to compute the probs matrix initially
 * And a void* params parameter used to hold a specific set of parameters for a given model
 */
typedef struct trade_impact_prob_model {
	void (*compute_probs)(double* probs, int N, void* params);
	void* params;
} trade_impact_prob_model;

/**
 *  This is an example of a structure used to represent a specific set of parameters for a given model
 */
typedef struct prob_squareinv_prm {
	// empty (no parameters)
} prob_squareinv_prm;
void compute_probs_squareinv(double* probs, int N, void* params);
typedef struct prob_deterministic_prm {
	// empty (no parameters)
} prob_deterministic_prm;
void compute_probs_deterministic(double* probs, int N, void* params);


#ifdef __cplusplus
}
#endif

#endif

