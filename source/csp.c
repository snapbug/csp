/*
	CSP.C
	-----
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
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
	uint64_t position_up_to, last_presented_and_seen, number_seen;
	uint64_t count, user, item, presented, rating, i, size;
	uint32_t *coraters = NULL;
	int64_t index;
	double last_prediction_error;
	double *error_presented, *error_rated, *auc;
	
	params->parse();
	stats = new CSP_stats(params->stats);
	dataset = new CSP_dataset_netflix(params);
	
	error_presented = new double[dataset->number_items];
	error_rated = new double[dataset->number_items];
	auc = new double[dataset->number_users];
	
	if (params->generation_method == CSP_generator_factory::BAYESIAN || params->prediction_method == CSP_predictor_factory::KORBELL)
	{
		coraters = new uint32_t[(tri_offset(dataset->number_items - 2, dataset->number_items - 1)) + 1];
		fprintf(stderr, "Loading coraters from file... "); fflush(stdout);
		size = fread(coraters, sizeof(*coraters), tri_offset(dataset->number_items - 2, dataset->number_items - 1) + 1, fopen("./data/netflix.coraters.item","rb"));
		fprintf(stderr, "Done.\n"); fflush(stdout);
	}
	
	/*
		Set the error accumulators to 0.
	*/
	for (item = 0; item < dataset->number_items; item++)
		error_rated[item] = error_presented[item] = 0;
	
	switch (params->generation_method)
	{
		//case CSP_generator_factory::BAYESIAN: generator = new CSP_generator_naive_bayes(dataset, coraters); break;
		//case CSP_generator_factory::ENTROPY: generator = new CSP_generator_entropy(dataset); break;
		//case CSP_generator_factory::ITEM_AVERAGE: generator = new CSP_generator_item_avg(dataset); break;
		//case CSP_generator_factory::RANDOM: generator = new CSP_generator_random(dataset); break;
		case CSP_generator_factory::POPULARITY: generator = new CSP_generator_popularity(dataset); break;
		default: exit(puts("Unknown generation method"));
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
	
	metric = new CSP_metric_mae(dataset, predictor);
	presentation_list = new uint64_t[dataset->number_items];
	
	/*
		For each user we're simulating a coldstart for. (Initial testee = 168)
	*/
	//#pragma omp parallel for private(user, position_up_to, last_presented_and_seen, number_seen, presented, presentation_list, key, last_prediction_error, ratings, count, rating) firstprivate(generator) schedule(dynamic, 500)
	//for (index = 0; index < (int64_t)dataset->number_users; index++)
	//if (false)
	index = 0;
	{
		user = index;
//		if (user % 100 == 0) { fprintf(stderr, "\r%6lu", user); fflush(stderr); }
		
		/*
			Reset things for this user.
		*/
		position_up_to = last_presented_and_seen = number_seen = presented = 0;
		key = NULL;
		auc[user] = last_prediction_error = 0;
		
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
		if (stats->stats & CSP_stats::ERROR_RATED)
			error_rated[number_seen] += last_prediction_error;
		
		/*
			While the user can still add more ratings.
		*/
		while (number_seen < count)
		{
			/*
				Generate the list of movies to present to the user.
			*/
			generator->generate(user, presentation_list, position_up_to);
			
			/*
				Find the next rating that the person can rate.
			*/
			for (presented = position_up_to; presented < dataset->number_items; presented++)
			{
				if (stats->stats & CSP_stats::ERROR_PRESENTED)
					error_presented[presented] += last_prediction_error;
				
				if ((key = (uint64_t *)bsearch(&presentation_list[presented], ratings, count, sizeof(*ratings), movie_search)) != NULL)
				{
					if (stats->stats & CSP_stats::AUC)
						auc[user] += ((1.0 * presented / dataset->number_items) - (1.0 * last_presented_and_seen / dataset->number_items)) * (1.0 * number_seen / count);
					number_seen++;
					
					/*
						Add the rating we came across.
					*/
					dataset->add_rating(key);
					predictor->added_rating(key);
					
					/*
						Make a note of where we are up to.
					*/
					last_presented_and_seen = presented++;
					position_up_to = presented;
					
					/*
						Now check our predictions on the test set for this user.
					*/
					if (stats->stats & CSP_stats::ERROR_PRESENTED || stats->stats & CSP_stats::ERROR_RATED)
						last_prediction_error = metric->score(user);
					if (stats->stats & CSP_stats::ERROR_RATED)
						error_rated[number_seen] += last_prediction_error;
					
					/*
						Stop looking for the next rating so we can re-generate presentation list.
					*/
					break;
				}
			}
		}
		
		/*
			Update the AUC for the presentation list, and print it out.
		*/
		if (stats->stats & CSP_stats::AUC)
			printf("AUC %lu %f\n", user, auc[user] + (1 - (1.0 * last_presented_and_seen / dataset->number_items)));
	
		/*
			Fill in the 'missing' values to give smooth graphs.
		*/
		for (item = number_seen + 1; stats->stats & CSP_stats::ERROR_RATED && item < dataset->number_items; item++)
			error_rated[item] += last_prediction_error;
		
		for (item = presented; stats->stats & CSP_stats::ERROR_PRESENTED && item < dataset->number_items; item++)
			error_presented[item] += last_prediction_error;
	}
	
	for (i = 0; stats->stats & CSP_stats::ERROR_RATED && i < dataset->number_items; i++)
		printf("ER %lu %f\n", i, error_rated[i] / dataset->number_users);
	for (i = 0; stats->stats & CSP_stats::ERROR_PRESENTED && i < dataset->number_items; i++)
		printf("EP %lu %f\n", i, error_presented[i] / dataset->number_users);
	
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
