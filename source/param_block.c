/*
	PARAM_BLOCK.C
	-------------
*/
#include <stdio.h>
#include <stdlib.h>
#include "csp_types.h"
#include "generator_factory.h"
#include "predictor_factory.h"
#include "metric_factory.h"
#include "stats.h"
#include "param_block.h"

/*
	CSP_PARAM_BLOCK::CSP_PARAM_BLOCK()
	----------------------------------
*/
CSP_param_block::CSP_param_block(int argc, char **argv)
{
	this->argc = argc;
	this->argv = argv;
	
	load_extra = TRUE;
	dataset_chosen = D_NETFLIX;
	testing_method = S_PROP;
	generation_method = CSP_generator_factory::POPULARITY;
	prediction_method = CSP_predictor_factory::USER_AVERAGE;
	metrics_to_use = CSP_metric_factory::MAE;
	stats = CSP_stats::NONE;
}

/*
	CSP_PARAM_BLOCK::USAGE()
	------------------------
*/
void CSP_param_block::usage(void)
{
	printf("Usage: %s [options...] <users>\n", argv[0]);
	printf("     : -h for help\n");
	
	exit(EXIT_SUCCESS);
}

/*
	CSP_PARAM_BLOCK::HELP()
	-----------------------
*/
void CSP_param_block::help(void)
{
	puts("CSP");
	puts("===");
	puts("Copyright Matt Crane 2010");
	puts("");
	
	puts("GENERAL");
	puts("-------");
	puts("-h               Display this help message");
	puts("");
	
	puts("DATASETS");
	puts("--------");
	puts("-d[mnN]          Use the following dataset for testing:");
	puts("   m             MovieLens 10M");
	puts("   n             Netflix [default]");
	puts("   N             Netflix Original");
	puts("-e               Don't load extra data (same data sorted by movie)");
	puts("");
	
	puts("TESTING");
	puts("-------");
	puts("-t[fp]           Test using:");
	puts("   f             Fixed testing set for each user");
	puts("   p             Proportional testing set for each user [default]");
	puts("");
	
	puts("GENERATION");
	puts("----------");
	puts("-g[bdegioOpPrt]  Generate lists to present to the user using:");
	puts("   b             Naive Bayes (dynamic)");
	puts("   d             Average distance from mean");
	puts("   e             Entropy0");
	puts("   g             Greedy Cheat (not to be used for real experiments)");
	puts("   i             Average");
	puts("   o             Other greedy");
	puts("   O             Other greedy perturbed");
	puts("   p             Popularity [default]");
	puts("   P             Use the predictor");
	puts("   r             Random");
	puts("   t             Tree (Other greedy with split decisions");
	puts("");
	
	puts("PREDICTION");
	puts("----------");
	puts("-p[cgiIkruU]     Use the following method for rating prediction:");
	puts("   c             Constant");
	puts("   g             Global average");
	puts("   i             Item average");
	puts("   I             Item k-nearest neighbour");
	puts("   k             KorBell");
	puts("   r             Random");
	puts("   u             User average [default]");
	puts("   U      *      User k-nearest neighbour");
	puts("");
	
	puts("METRICS");
	puts("-------");
	puts("-m[nmMrR]        Calculate performance of predictions using:");
	puts("   n             None");
	puts("   m             Mean Absolute Error [default]");
	puts("   M             Normalized Mean Absolute Error");
	puts("   r             Root Mean Squared Error");
	puts("   R             Normalized Root Mean Squared Error");
	puts("");
	
	puts("STATISTICS");
	puts("----------");
	puts("-s[naAeEfht]     Measure and output the following statistics:");
	puts("   n             None [default]");
	puts("   a             All statistics");
	puts("   A             AUC for presentation lists (number rated vs number presented)");
	puts("   e             Error as function of number presented");
	puts("   E             Error as function of number rated");
	puts("   f             Finish time (how long till we get all ratings)");
	puts("   h             Hit Rate (when do we get a movie to rate)");
	puts("   t             Ttest (output prediction error per person every 25 presents");
	
	exit(EXIT_SUCCESS);
}

/*
	CSP_PARAM_BLOCK::DATASET()
	--------------------------
*/
void CSP_param_block::dataset(char *which)
{
	dataset_chosen = D_NONE;
	switch (*which)
	{
		case 'm': dataset_chosen = D_MOVIELENS; break;
		case 'n': dataset_chosen = D_NETFLIX; break;
		case 'N': dataset_chosen = D_NETFLIX_ORIG; break;
		default: exit(printf("Unknown dataset: '%c'\n", *which));
	}
}

/*
	CSP_PARAM_BLOCK::GENERATION()
	-----------------------------
*/
void CSP_param_block::generation(char *which)
{
	switch (*which)
	{
		case 'b': generation_method = CSP_generator_factory::BAYESIAN; break;
		case 'd': generation_method = CSP_generator_factory::DISTANCE; break;
		case 'e': generation_method = CSP_generator_factory::ENTROPY; break;
		case 'g': generation_method = CSP_generator_factory::GREEDY_CHEAT; break;
		case 'i': generation_method = CSP_generator_factory::ITEM_AVERAGE; break;
		case 'p': generation_method = CSP_generator_factory::POPULARITY; break;
		case 'P': generation_method = CSP_generator_factory::PREDICTOR; break;
		case 'r': generation_method = CSP_generator_factory::RANDOM; break;
		case 't': generation_method = CSP_generator_factory::TREE; break;
		case 'o': generation_method = CSP_generator_factory::OTHER_GREEDY; break;
		case 'O': generation_method = CSP_generator_factory::OTHER_GREEDY_PERS; break;
		default: exit(printf("Unknown generation method: '%c'\n", *which));
	}
}

/*
	CSP_PARAM_BLOCK::PREDICTION()
	-----------------------------
*/
void CSP_param_block::prediction(char *which)
{
	switch (*which)
	{
		case 'c': prediction_method = CSP_predictor_factory::CONSTANT; break;
		case 'g': prediction_method = CSP_predictor_factory::GLOBAL_AVERAGE; break;
		case 'i': prediction_method = CSP_predictor_factory::ITEM_AVERAGE; break;
		case 'I': prediction_method = CSP_predictor_factory::ITEM_ITEM_KNN; break;
		case 'k': prediction_method = CSP_predictor_factory::KORBELL; break;
		case 'r': prediction_method = CSP_predictor_factory::RANDOM; break;
		case 'u': prediction_method = CSP_predictor_factory::USER_AVERAGE; break;
		case 'U': prediction_method = CSP_predictor_factory::USER_USER_KNN; break;
		default: exit(printf("Unknown prediction method: '%c'\n", *which));
	}
}

/*
	CSP_PARAM_BLOCK::METRICS()
	--------------------------
*/
void CSP_param_block::metrics(char *which)
{
//	do
//	{
		switch (*which)
		{
			case 'n': metrics_to_use = CSP_metric_factory::NONE; break;
			case 'm': metrics_to_use = CSP_metric_factory::MAE; break;
			case 'M': metrics_to_use = CSP_metric_factory::NMAE; break;
			case 'r': metrics_to_use = CSP_metric_factory::RMSE; break;
			case 'R': metrics_to_use = CSP_metric_factory::NRMSE; break;
			default: exit(printf("Unknown metric: '%c'\n", *which));
		}
//		which++;
//	}
//	while (*which != '\0');
}

/*
	CSP_PARAM_BLOCK::STATISTICS()
	-----------------------------
*/
void CSP_param_block::statistics(char *which)
{
	do
	{
		switch (*which)
		{
			case 'n': stats = CSP_stats::NONE; break;
			case 'a': statistics("AeEfghpt"); break;
			case 'A': stats |= CSP_stats::AUC; break;
			case 'e': stats |= CSP_stats::ERROR_PRESENTED; break;
			case 'E': stats |= CSP_stats::ERROR_RATED; break;
			case 'f': stats |= CSP_stats::FINISH; break;
			case 'g': stats |= CSP_stats::GENERATION_TIME; break;
			case 'h': stats |= CSP_stats::HIT_RATE; break;
			case 'p': stats |= CSP_stats::PREDICTION_TIME; break;
			case 't': stats |= CSP_stats::TTEST; break;
			default: exit(printf("Unknown statistic: '%c'\n", *which));
		}
		which++;
	}
	while (*which != '\0');
}

/*
	CSP_PARAM_BLOCK::PARSE()
	------------------------
*/
uint64_t CSP_param_block::parse(void)
{
	uint64_t param;
	char *command;
	
	for (param = 1; param < (uint64_t)argc; param++)
	{
		if (*argv[param] == '-')
		{
			command = argv[param] + 1;
			if (*command == 't')
			{
				if (*(command + 1) == 'p')
					testing_method = S_PROP;
				else
					testing_method = S_FIXED;
			}
			else if (*command == 'd')
				dataset(command + 1);
			else if (*command == 'e')
				load_extra = FALSE;
			else if (*command == 'g')
				generation(command + 1);
			else if (*command == 'p')
				prediction(command + 1);
			else if (*command == 'm')
				metrics(command + 1);
			else if (*command == 's')
				statistics(command + 1);
			else if (*command == 'h')
				help();
			else
				usage();
		}
		else
			break;
	}
	
	return param;
}
