#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <utl.h>

#include <tm_opt.h>
#include <tm_priceshift_models.h>
#include <tm_prob_models.h>


int testmain(int N, int T, trade_impact_priceshift_model* priceshift_model, trade_impact_prob_model* prob_model) {
	int ret = 0;

	trade_impact_problem* pb;

	int t;

	ret = trade_imp_pb_create(&pb, N, T, priceshift_model, prob_model);
	if (ret != 0) {
		fprintf(stderr, "clientmain(): error in trade_impact_problem_create()\n");
		return -1;
	}

	ret = trade_imp_pb_optimize(pb, 1);
	if (ret < 0) {
		fprintf(stderr, "clientmain(): error in optimize_trade()\n");
		return -1;
	}

	for (t = 0; t <= pb->T-1; t++) {
		//printf("F(t=%d,.): ", t); UTLShowVector(pb->N+1, &F(pb, t, 0));
	}
	for (t = 0; t <= pb->T-1; t++) {
		//printf("path matrix(t=%d,.): ", t); UTLShowIntVector(pb->N+1, &FP(pb, t, 0));
	}

	printf("Optimal value for trade sequencing = %g\n", F(pb, 0, pb->N));


	trade_imp_pb_fwprop_deterministic(pb, 2, NULL, NULL);

	ret = trade_imp_pb_delete(&pb);


	return 0;
}


void print_usage(char* argv0) {
	printf("Usage: %s <N> <T> <priceshift model: alphapi/alphalog> <parameters> <prob model: squareinv/deterministic> <parameters>\n", argv0);
}

int main(int argc, char** argv) {
	int ret = 0;

	int i;
	int parse_error;

	int N;
	int T;
	//int verbose;


	trade_impact_priceshift_model priceshift_model;
	trade_impact_prob_model prob_model;
	priceshift_alphalog_prm priceshift_alphalog_params;
	priceshift_alphapi_prm priceshift_alphapi_params;
	prob_squareinv_prm prob_default_params;
	prob_deterministic_prm prob_deterministic_params;


	// Default parameters
	N = 1000;
	T = 10;
	priceshift_alphalog_params.alpha = 0.15;
	priceshift_alphapi_params.alpha = 0.001;
	priceshift_alphapi_params.pi = 0.9;




	//verbose = 0;


	parse_error = 0;
	i = 0;

	i++;
	if (argc <= i) {
		parse_error = 1; print_usage(argv[0]); return -1;
	}
	N = atoi(argv[i]);
	i++;
	if (argc <= i) {
		parse_error = 1; print_usage(argv[0]); return -1;
	}
	T = atoi(argv[i]);
	i++;
	if (argc <= i) {
		parse_error = 1; print_usage(argv[0]); return -1;
	}
	if (!strcmp(argv[i], "alphapi")) {
		i++;
		if (argc <= i) {
			parse_error = 1; print_usage(argv[0]); return -1;
		}
		priceshift_alphapi_params.alpha = atof(argv[i]);
		i++;
		if (argc <= i) {
			parse_error = 1; print_usage(argv[0]); return -1;
		}
		priceshift_alphapi_params.pi = atof(argv[i]);

		priceshift_model.compute_priceshifts = compute_priceshifts_alphapi;
		priceshift_model.params = (void*)&priceshift_alphapi_params;
	}
	else if (!strcmp(argv[i], "alphalog")) {
		i++;
		if (argc <= i) {
			parse_error = 1; print_usage(argv[0]); return -1;
		}
		priceshift_alphalog_params.alpha = atof(argv[i]);

		priceshift_model.compute_priceshifts = compute_priceshifts_alphalog;
		priceshift_model.params = (void*)&priceshift_alphalog_params;

	}
	else {
		parse_error = 1; print_usage(argv[0]); return -1;
	}
	i++;
	if (argc <= i) {
		parse_error = 1; print_usage(argv[0]); return -1;
	}
	if (!strcmp(argv[i], "squareinv")) {
		prob_model.compute_probs = compute_probs_squareinv;
		prob_model.params = (void*)&prob_default_params;
	}
	else if (!strcmp(argv[i], "deterministic")) {
		prob_model.compute_probs = compute_probs_deterministic;
		prob_model.params = (void*)&prob_deterministic_params;

	}
	else {
		parse_error = 1; print_usage(argv[0]); return -1;
	}
	if (parse_error) {
		print_usage(argv[0]);
	}

	ret = testmain(N, T, &priceshift_model, &prob_model);


	return ret;
}

