#ifndef TMWOPT_H
#define TMWOPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tmw_priceshift_models.h"
#include "tmw_prob_models.h"


typedef struct trade_impact_problem {
	int N;
	int T;
	double* optimal; // F
	int* path;
	double* probs;
	double* d;
	trade_impact_priceshift_model* priceshift_model;
	trade_impact_prob_model* prob_model;
} trade_impact_problem;

/*
 * probs is a (N+1)x(N+1) matrix representing prob(k'|k)
 * 0<=k<=N
 * 0<=kp<=k
 */
#define prob(pb, kp, k) pb->probs[(k)*(pb->N+1) + (kp)]
/*
 * F (optimal) is a Tx(N+1) matrix representing F. Here F(t, n) represents F(t+1, n) in the pdf writeup
 * 0<=t<=T-1
 * 0<=n<=N
 */
#define F(pb, t, n) pb->optimal[(t)*(pb->N+1) + (n)]
/*
 * FP (path) is a Tx(N+1) integer matrix representing the optimal path followed to compute F
 * 0<=t<=T-1
 * 0<=n<=N
 */
#define FP(pb, t, n) pb->path[(t)*(pb->N+1) + (n)]


int trade_imp_pb_create(trade_impact_problem** ppb, int N, int T, trade_impact_priceshift_model* priceshift_model, trade_impact_prob_model* prob_model);
int trade_imp_pb_delete(trade_impact_problem** ppb);
double trade_imp_pb_fw_prop_deterministic(trade_impact_problem* pb, int verbose, int* optimal_sells, double* optimal_prices);
int trade_imp_pb_optimize(trade_impact_problem* pb, int verbose);




#ifdef __cplusplus
}
#endif

#endif

