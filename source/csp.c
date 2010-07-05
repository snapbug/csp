/*
	CSP.C
	-----
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <omp.h>
#include "csp_types.h"
#include "dataset_netflix.h"
#include "dataset_netflix_orig.h"
#include "generator_factory.h"
#include "predictor_factory.h"
#include "param_block.h"
#include "metric_factory.h"
#include "metric_mae.h"
#include "metric_rmse.h"
#include "stats.h"

int movie_search(const void *a, const void *b)
{
	uint64_t key = *(uint64_t *)a;
	uint64_t item = (*(uint64_t *)b) >> 15 & 32767;
	return (key > item) - (key < item);
}

int main(int argc, char **argv)
{
	CSP_param_block *params = new CSP_param_block(argc, argv);
	CSP_generator *generator;
	CSP_dataset *dataset;
	CSP_predictor *predictor;
	CSP_stats *stats;
	CSP_metric *metric;
	uint64_t *presentation_list, *key, *ratings;
	uint64_t position_up_to, last_presented_and_seen, number_seen, count, user, item, presented, rating, size;
	uint32_t *coraters = NULL;
	uint64_t last_param;
	double last_prediction_error, auc;
	double *error_presented, *error_rated;
	uint64_t *count_presented, *count_rated;
	
	last_param = params->parse();
	stats = new CSP_stats(params->stats);
	dataset = new CSP_dataset_netflix(params);
	
	/*
		Load the precalculated co-raters if necessary.
	*/
	if (params->generation_method == CSP_generator_factory::BAYESIAN || params->prediction_method == CSP_predictor_factory::KORBELL)
	{
		coraters = new uint32_t[(tri_offset(dataset->number_items - 2, dataset->number_items - 1)) + 1];
		fprintf(stderr, "Loading coraters from file... "); fflush(stderr);
		size = fread(coraters, sizeof(*coraters), tri_offset(dataset->number_items - 2, dataset->number_items - 1) + 1, fopen("./data/netflix.coraters.item","rb"));
		fprintf(stderr, "Done.\n"); fflush(stderr);
	}
	
	switch (params->prediction_method)
	{
		case CSP_predictor_factory::CONSTANT: predictor = new CSP_predictor_constant(dataset); break;
		case CSP_predictor_factory::GLOBAL_AVERAGE: predictor = new CSP_predictor_global_avg(dataset); break;
		case CSP_predictor_factory::ITEM_AVERAGE: predictor = new CSP_predictor_item_avg(dataset); break;
		case CSP_predictor_factory::ITEM_ITEM_KNN: predictor = new CSP_predictor_item_knn(dataset, 20); break;
		case CSP_predictor_factory::KORBELL: predictor = new CSP_predictor_korbell(dataset, 20, coraters); break;
		case CSP_predictor_factory::RANDOM: predictor = new CSP_predictor_random(dataset); break;
		case CSP_predictor_factory::USER_AVERAGE: predictor = new CSP_predictor_user_avg(dataset); break;
		case CSP_predictor_factory::USER_USER_KNN: predictor = new CSP_predictor_user_knn(dataset, 20); break;
		default: exit(puts("Unknown prediction method"));
	}
	
	metric = new CSP_metric_rmse(dataset, predictor);
	
	switch (params->generation_method)
	{
		case CSP_generator_factory::BAYESIAN: generator = new CSP_generator_naive_bayes(dataset, coraters); break;
		case CSP_generator_factory::ENTROPY: generator = new CSP_generator_entropy(dataset); break;
		case CSP_generator_factory::GREEDY_CHEAT: generator = new CSP_generator_greedy_cheat(dataset, predictor, metric); break;
		case CSP_generator_factory::ITEM_AVERAGE: generator = new CSP_generator_item_avg(dataset); break;
		case CSP_generator_factory::RANDOM: generator = new CSP_generator_random(dataset); break;
		case CSP_generator_factory::POPULARITY: generator = new CSP_generator_popularity(dataset); break;
		default: exit(puts("Unknown generation method"));
	}
	
	error_presented = new double[dataset->number_items + 1];
	error_rated = new double[dataset->number_items + 1];
	count_presented = new uint64_t[dataset->number_items + 1];
	count_rated = new uint64_t[dataset->number_items + 1];
	
	for (item = 0; item <= dataset->number_items; item++)
	{
		error_presented[item] = error_rated[item] = 0;
		count_presented[item] = count_rated[item] = 0;
	}
	
	/*
		For each user we're simulating a coldstart for. (Initial testee = 168)
	*/
	for (; last_param < (uint64_t)argc; last_param++)
	//for (user = 0; user < dataset->number_users; user++)
	{
		user = strtoull(argv[last_param], (char **)NULL, 10);
		//if (user % 100 == 0) { fprintf(stderr, "\r%6lu", user); fflush(stderr); }
		printf("%lu ", user);
		
		/*
			Reset things for this user.
		*/
		position_up_to = last_presented_and_seen = number_seen = presented = 0;
		key = NULL;
		last_prediction_error = auc = 0;
		
		/*
			Get the ratings for this user, and then remove them all from the dataset.
		*/
		ratings = dataset->ratings_for_user(user, &count);
		for (rating = 0; rating < count; rating++)
		{
			dataset->remove_rating(&ratings[rating]);
			predictor->removed_rating(&ratings[rating]);
		}
		
		/*
			Before we add any ratings, we should see how well we can do.
		*/
		if (stats->stats & CSP_stats::ERROR_PRESENTED || stats->stats & CSP_stats::ERROR_RATED)
			last_prediction_error = metric->score(user);
		if (stats->stats & CSP_stats::ERROR_RATED && !isnan(last_prediction_error))
		{
			error_rated[number_seen] += last_prediction_error;
			count_rated[number_seen]++;
		}
		if (stats->stats & CSP_stats::ERROR_PRESENTED && !isnan(last_prediction_error))
		{
			error_presented[presented] += last_prediction_error;
			count_presented[presented]++;
		}
		
		/*
			While the user can still add more ratings.
		*/
		while (number_seen < count)
		{
			if (number_seen % 10 == 0) { fprintf(stderr, "\r%6lu%6lu/%6lu", user, number_seen, count); fflush(stderr); }
			/*
				Generate the list of movies to present to the user.
			*/
			presentation_list = generator->generate(user, position_up_to);
			
			/*
				Find the next rating that the person can rate.
			*/
			for (presented = position_up_to; presented < dataset->number_items; presented++)
			{
				if ((key = (uint64_t *)bsearch(&presentation_list[presented], ratings, count, sizeof(*ratings), movie_search)) != NULL)
				{
					if (stats->stats & CSP_stats::AUC)
						auc += ((1.0 * presented / dataset->number_items) - (1.0 * last_presented_and_seen / dataset->number_items)) * (1.0 * number_seen / count);
					number_seen++;
					
					/*
						Add the rating we came across.
					*/
					dataset->add_rating(key);
					predictor->added_rating(key);
					
					/*
						Now check our error for this user.
					*/
					if (stats->stats & CSP_stats::ERROR_PRESENTED || stats->stats & CSP_stats::ERROR_RATED)
						last_prediction_error = metric->score(user);
					
					/*
						Update the error as function of number rated.
					*/
					if (stats->stats & CSP_stats::ERROR_RATED)
					{
						error_rated[number_seen] += last_prediction_error;
						count_rated[number_seen]++;
					}
				}
				
				/*
					Update the error as function of number presented.
				*/
				if (stats->stats & CSP_stats::ERROR_PRESENTED)
				{
					error_presented[presented + 1] += last_prediction_error;
					count_presented[presented + 1]++;
				}
				
				if (key != NULL)
				{
					/*
						Make a note of where we are up to.
					*/
					last_presented_and_seen = presented++;
					position_up_to = presented;
					
					/*
						Stop looking for the next rating so we can re-generate presentation list.
					*/
					break;
				}
			}
		}
		
		/*
			Print out the AUC for this user for this presentation list.
		*/
		if (stats->stats & CSP_stats::AUC)
			printf("A %lu %f\n", user, auc + (1 - (1.0 * last_presented_and_seen / dataset->number_items)));
	
		/*
			Fill in the 'missing' values to give smooth graphs.
		*/
		for (item = number_seen + 1; item <= dataset->number_items; item++)
			if (stats->stats & CSP_stats::ERROR_RATED)
			{
				error_rated[item] += last_prediction_error;
				count_rated[item]++;
			}
		for (item = presented + 1; item <= dataset->number_items; item++)
			if (stats->stats & CSP_stats::ERROR_PRESENTED)
			{
				error_presented[item] += last_prediction_error;
				count_presented[item]++;
			}
	}
	
	printf("\n");
	if (stats->stats & CSP_stats::ERROR_PRESENTED)
		for (item = 0; item <= dataset->number_items; item++)
			printf("P %lu %f\n", item, error_presented[item] / count_presented[item]);
	if (stats->stats & CSP_stats::ERROR_RATED)
		for (item = 0; item <= dataset->number_items; item++)
			printf("R %lu %f\n", item, error_rated[item] / count_rated[item]);
	
	/*
		Clean up.
	*/
	delete params;
	delete generator;
	delete dataset;
	delete predictor;
	delete metric;
	delete stats;
	
	return EXIT_SUCCESS;
}
