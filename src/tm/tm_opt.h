#ifndef TMOPT_H
#define TMOPT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "tm_priceshift_models.h"
#include "tm_prob_models.h"


/**
 * This structure holds the main variables describing a trade impact model problem
 */
typedef struct trade_impact_problem {
	int N; // number of shares
	int T; // number of periods
	double* optimal;
	/*
	 optimal (F) is a Tx(N+1) matrix of F(t,n): Maximum expected revenue if we have n shares to sell at time t
	 * 0<=t<=T-1
	 * 0<=n<=N
	 */
	int* path;
	/*
	 * FP (path) is a Tx(N+1) integer matrix representing the optimal path followed to compute F
	 * 0<=t<=T-1
	 * 0<=n<=N
	 */
	double* probs;
	/*
	 * probs is a (N+1)x(N+1) matrix representing prob(k'|k) 0<=k<=N,0<=kp<=k
	 * The probability to sell k' shares given we wish to sell k
	 */
	double* d;
	/**
	 * d is the (N+1)x1 vector of price shifts for every number k of shares announced
	 */
	trade_impact_priceshift_model* priceshift_model;
	/**
	 *  This is the model used to compute the price shifts vector d
	 */
	trade_impact_prob_model* prob_model;
	/**
	 * This is the model used to compute the probabilistic model probs
	 */
} trade_impact_problem;

/**
 * The following macros are used to make the code easier to read
 */
#define prob(pb, kp, k) pb->probs[(k)*(pb->N+1) + (kp)]
#define F(pb, t, n) pb->optimal[(t)*(pb->N+1) + (n)]
#define FP(pb, t, n) pb->path[(t)*(pb->N+1) + (n)]

/**
 * Read tm_opt.h for more info on the functions
 */
int trade_imp_pb_create(trade_impact_problem** ppb, int N, int T, trade_impact_priceshift_model* priceshift_model, trade_impact_prob_model* prob_model);
int trade_imp_pb_delete(trade_impact_problem** ppb);
int trade_imp_pb_optimize(trade_impact_problem* pb, int verbose);
double trade_imp_pb_fwprop_deterministic(trade_impact_problem* pb, int verbose, int* optimal_sells, double* optimal_prices);





#ifdef __cplusplus
}
#endif

#endif

