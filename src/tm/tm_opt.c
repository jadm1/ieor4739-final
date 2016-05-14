
#include "tm_opt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <utl.h>



/**
 *  This function creates a trade impact model problem
 *  It allocates all structures from the specified parameters
 */
int trade_imp_pb_create(trade_impact_problem** ppb, int N, int T, trade_impact_priceshift_model* priceshift_model, trade_impact_prob_model* prob_model) {
	trade_impact_problem* pb;
	int ret;
	int k;

	pb = (trade_impact_problem*)calloc(1, sizeof(trade_impact_problem));
	if (pb == NULL) {
		fprintf(stderr, "trade_imp_pb_create(): error not enough memory\n");
		return -1;
	}

	printf("N: %d, T: %d\n", N, T);

	pb->N = N;
	pb->T = T;

	pb->probs = (double*)calloc((N+1)*(N+1), sizeof(double));
	pb->optimal = (double*)calloc(T*(N+1), sizeof(double));
	pb->path = (int*)calloc(T*(N+1), sizeof(int));
	pb->d = (double*)calloc(N+1, sizeof(double));
	if (pb->probs == NULL || pb->optimal == NULL || pb->path == NULL || pb->d == NULL) {
		fprintf(stderr, "trade_imp_pb_create(): error not enough memory\n");
		return -1;
	}

	// Initialize models
	pb->priceshift_model = priceshift_model;
	pb->prob_model = prob_model;

	pb->priceshift_model->compute_priceshifts(pb->d, pb->N, pb->priceshift_model->params);
	pb->prob_model->compute_probs(pb->probs, pb->N, pb->prob_model->params);

	for (k = 0; k <= N; k++) {
		//printf("prob(.,%d): ", k); UTLShowVector(k+1, &prob(pb, 0, k));
	}

	//printf("d(.): "); UTLShowVector(N+1, pb->d);


	*ppb = pb;
	return 0;
}

/**
 * Delete a trade impact model problem
 *  Frees everything
 */
int trade_imp_pb_delete(trade_impact_problem** ppb) {
	trade_impact_problem* pb;

	if (ppb == NULL) {
		return 0;
	}

	pb = *ppb;


	free((void*)pb->d);
	free((void*)pb->path);
	free((void*)pb->optimal);
	free((void*)pb->probs);

	pb = NULL;
	*ppb = pb;
	return 0;
}


/**
 *  This is the main function used to build the optimal F matrix dynamically
 *  It starts by filling the last stage (t=T-1) and then fills all other stages going backwards
 */

int trade_imp_pb_optimize(trade_impact_problem* pb, int verbose) {
	int ret = 0;
	double expected_revenue;
	double best_expected_revenue;
	int best_index;

	double profit;
	double newprice;
	int sell;

	int N = pb->N;
	int T = pb->T;

	int n, t;
	int k, kp;

	/*
	 * Fill the last step
	 */
	t = T-1;
	if (verbose >= 2) {
		printf("t = %d\n", t); fflush(stdout);
	}
	best_expected_revenue = 0.0;
	for (n = 0; n <= N; n++) {
		expected_revenue = 0.0;
		k = n; // trick because the quantity we are maximizing does not depend on n
		for (kp = 0; kp <= k; kp++) {
			expected_revenue += prob(pb, kp, k) * (double)kp;
		}
		expected_revenue *= pb->d[k];
		if (expected_revenue > best_expected_revenue) {
			best_expected_revenue = expected_revenue;
		}
		F(pb, t, n) = best_expected_revenue;
		FP(pb, t, n) = n;
	}

	//printf("F(t=%d,.): ", T-1); UTLShowVector(N+1, &F(pb, t, 0)); // print some debug info

	/*
	 * propagate backwards
	 */
	for (t = T-2; t >= 0; t--) {
		if (verbose >= 2) {
			printf("t = %d\n", t); fflush(stdout);
		}
		for (n = 0; n <= N; n++) {
			best_expected_revenue = 0.0;
			best_index = 0;
			for (k = 0; k <= n; k++) {
				// compute the sum
				expected_revenue = 0.0;
				for (kp = 0; kp <= k; kp++) {
					expected_revenue += prob(pb, kp, k) * ((double)kp + F(pb, t + 1, n - kp));
				}
				expected_revenue *= pb->d[k];
				// update if best
				if (expected_revenue > best_expected_revenue) {
					best_expected_revenue = expected_revenue;
					best_index = k;
				}
			}
			// here we know the max
			F(pb, t, n) = best_expected_revenue;
			FP(pb, t, n) = best_index;
		}
	}


	return 0;
}




/**
 * Once trade_imp_pb_optimize() has been called and F has been built,
 * This function shows how the optimal trading sequence can be performed by propagating forwards this time
 * It optionally returns a sequence of optimal sell quantities and the corresponding resuting prices
 */
double trade_imp_pb_fwprop_deterministic(trade_impact_problem* pb, int verbose, int* optimal_sells, double* optimal_prices) {
	// forward propagation (used to check path is correct)
	int N = pb->N;
	int T = pb->T;

	int t;
	double profit, newprice;
	int sell, left;


	left = N;

	profit = 0.0;
	newprice = 1.0;
	for (t = 0; t <= T-1; t++) {
		sell = FP(pb, t, left);

		if (optimal_sells != NULL) {
			optimal_sells[t] = sell;
		}

		if (verbose >= 1)
			printf("at step %d: optimal: %g, sell: %d", t, F(pb, t, left), sell);

		left = left - sell;

		if (verbose >= 1)
			printf(", left: %d", left);

		// compute profit along the path
		newprice *= pb->d[sell];

		if (optimal_prices != NULL) {
			optimal_prices[t] = newprice;
		}

		if (verbose >= 2) {
			printf(", price: %g", newprice);
			printf(", sale value: %g", newprice * (double)sell);
		}

		profit += newprice * (double)sell;

		if (verbose >= 1)
			printf("\n");
	}

	if (verbose >= 1)
		printf("Final profit best case: %g\n", profit);

	return profit;
}


