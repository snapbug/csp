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
#include "dataset_movielens.h"
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
#ifdef ML
	uint64_t item = (*(uint64_t *)b) >> 4 & 16383;
#else
	uint64_t item = (*(uint64_t *)b) >> 15 & 32767;
#endif
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
	uint64_t *key, *ratings;
	uint64_t position_up_to, last_presented_and_seen, number_seen, count, user, item, presented, rating, size, next_movie;
	uint32_t *coraters = NULL;
	double last_prediction_error, auc;
	double *error_presented, *error_rated;
	uint64_t last_param, present_max, number_users;
	clock_t start, end;
	
	last_param = params->parse();
	number_users = argc - last_param;
	stats = new CSP_stats(params->stats);
	
	switch (params->dataset_chosen)
	{
		case CSP_param_block::D_NETFLIX: dataset = new CSP_dataset_netflix(params); break;
		case CSP_param_block::D_NETFLIX_ORIG: dataset = new CSP_dataset_netflix_orig(params); break;
		case CSP_param_block::D_MOVIELENS: dataset = new CSP_dataset_movielens(params); break;
		default: exit(fprintf(stderr, "Unknown dataset\n"));
	}
	
	if (stats->stats & (CSP_stats::FINISH | CSP_stats::HIT_RATE | CSP_stats::AUC))
		present_max = dataset->number_items;
	else
		present_max = 220;
	
	/*
		Load the precalculated co-raters if necessary.
	*/
	if (params->generation_method == CSP_generator_factory::BAYESIAN || params->generation_method == CSP_generator_factory::OTHER_GREEDY_PERS || params->prediction_method == CSP_predictor_factory::KORBELL)
	{
		coraters = new uint32_t[(tri_offset(dataset->number_items - 2, dataset->number_items - 1, dataset->number_items)) + 1];
		if (params->dataset_chosen == CSP_param_block::D_NETFLIX)
		{
			fprintf(stderr, "Loading coraters from file... "); fflush(stderr);
			size = fread(coraters, sizeof(*coraters), tri_offset(dataset->number_items - 2, dataset->number_items - 1, dataset->number_items) + 1, fopen("./data/netflix.coraters.item","rb"));
			fprintf(stderr, "done.\n"); fflush(stderr);
		}
		else
		{
			uint64_t i, j, k;
			for (i = 0; i < dataset->number_items; i++)
			{
				if (i % 100 == 0) { fprintf(stderr, "\r%5lu", i); fflush(stderr); }
				ratings = dataset->ratings_for_movie(i, &count);
				for (j = 0; j < count; j++)
				{
					key = dataset->ratings_for_user(dataset->user(ratings[j]), &user);
					for (k = 0; k < user; k++)
						if (i < dataset->movie(key[k]))
							coraters[tri_offset(i, dataset->movie(key[k]), dataset->number_items)]++;
				}
			}
			fwrite(coraters, sizeof(*coraters), tri_offset(dataset->number_items - 2, dataset->number_items - 1, dataset->number_items) + 1, fopen("./data/ml.100k.coraters.item", "wb"));
			fprintf(stderr, "\n");
		}
	}
	
	switch (params->prediction_method)
	{
		case CSP_predictor_factory::CONSTANT: predictor = new CSP_predictor_constant(dataset); break;
		case CSP_predictor_factory::GLOBAL_AVERAGE: predictor = new CSP_predictor_global_avg(dataset); break;
		case CSP_predictor_factory::ITEM_AVERAGE: predictor = new CSP_predictor_item_avg(dataset); break;
		case CSP_predictor_factory::ITEM_ITEM_KNN: predictor = new CSP_predictor_item_knn(dataset, 20); break;
		case CSP_predictor_factory::KORBELL: predictor = new CSP_predictor_korbell(dataset, 20, coraters, params); break;
		case CSP_predictor_factory::RANDOM: predictor = new CSP_predictor_random(dataset); break;
		case CSP_predictor_factory::USER_AVERAGE: predictor = new CSP_predictor_user_avg(dataset); break;
		case CSP_predictor_factory::USER_USER_KNN: predictor = new CSP_predictor_user_knn(dataset, 20); break;
		default: exit(fprintf(stderr, "Unknown prediction method\n"));
	}
	
	switch (params->metrics_to_use)
	{
		case CSP_metric_factory::MAE: metric = new CSP_metric_mae(dataset, predictor); break;
		case CSP_metric_factory::RMSE: metric = new CSP_metric_rmse(dataset, predictor); break;
		default: exit(fprintf(stderr, "Unknown metric\n"));
	}
	
	switch (params->generation_method)
	{
		case CSP_generator_factory::BAYESIAN: generator = new CSP_generator_naive_bayes(dataset, coraters); break;
		case CSP_generator_factory::DISTANCE: generator = new CSP_generator_distance(dataset); break;
		case CSP_generator_factory::ENTROPY: generator = new CSP_generator_entropy(dataset); break;
		case CSP_generator_factory::GREEDY_CHEAT: generator = new CSP_generator_greedy_cheat(dataset, predictor, metric); break;
		case CSP_generator_factory::OTHER_GREEDY: generator = new CSP_generator_other_greedy(dataset, predictor, metric); break;
		case CSP_generator_factory::OTHER_GREEDY_PERS: generator = new CSP_generator_other_greedy_pers(dataset, predictor, metric, coraters); break;
		case CSP_generator_factory::ITEM_AVERAGE: generator = new CSP_generator_item_avg(dataset); break;
		case CSP_generator_factory::RANDOM: generator = new CSP_generator_random(dataset); break;
		case CSP_generator_factory::POPULARITY: generator = new CSP_generator_popularity(dataset); break;
		case CSP_generator_factory::TREE: generator = new CSP_generator_tree(dataset, predictor, metric); break;
		case CSP_generator_factory::PREDICTOR: generator = new CSP_generator_predictor(dataset, predictor, metric); break;
		default: exit(fprintf(stderr, "Unknown generation method\n"));
	}
	
	error_presented = new double[present_max + 1];
	error_rated = new double[present_max + 1];
	
#if 0
	#define NUM 20
	uint64_t counts[NUM][5] = {0};
	uint64_t movies[] = {
		5316, 8781, 9339, 12231, 15204, 7634, 15123, 11148, 1144, 6385, 14312, 6286, 11282, 14239, 14643, 12316, 6971, 4995, 16376, 11520
		};
	uint64_t i, j, movie;
	
	// for each user
	for (user = 0; user < dataset->number_users; user++)
	{
		ratings = dataset->ratings_for_user(user, &count);
		// for each of the top 10 greedy movies
		for (movie = 0; movie < NUM; movie++)
		{
			// see if the movie was in this users top 10
			for (j = 0; j < NUMDONE; j++)
			{
				if (greedy_movies[(NUMDONE * user) + j] == movies[movie])
				{
					// get the rating they gave it
					key = (uint64_t *)bsearch(&movies[movie], ratings, count, sizeof(*ratings), movie_search);
					counts[movie][dataset->rating(key) - 1]++;
				}
			}
		}
	}
	
	for (i = 0; i < NUM; i++)
	{
		// first the greedy
		count = 0;
		for (j = 0; j < 5; j++)
			count += counts[i][j];
		for (j = 0; j < 5; j++)
		{
			printf("%f ", 1.0 * counts[i][j] / count);
			counts[i][j] = 0;
		}
		printf("\n");
		
		// then the population
		ratings = dataset->ratings_for_movie(movies[i], &count);
		for (j = 0; j < count; j++)
			counts[i][dataset->rating(ratings[j]) - 1]++;
		
		for (j = 0; j < 5; j++)
			printf("%f ", 1.0 * counts[i][j] / count);
		
		printf("\n");
	}
	
	return EXIT_SUCCESS;
#endif
	
	for (item = 0; item <= present_max; item++)
		error_presented[item] = error_rated[item] = 0;
	
	start = clock();
	/*
		For each user we're simulating a coldstart for.
	*/
	for (; last_param < (uint64_t)argc; last_param++)
	//for (user = 0; user < dataset->number_users; user++)
	{
		user = strtoull(argv[last_param], (char **)NULL, 10);
		
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
		if (stats->stats & (CSP_stats::ERROR_RATED | CSP_stats::ERROR_PRESENTED))
			last_prediction_error = metric->score(user);
		
		error_rated[number_seen] += last_prediction_error;
		error_presented[presented] += last_prediction_error;
		
		/*
			While the user can still add more ratings, and we can still present some.
		*/
		while (number_seen < count && presented <= present_max)
		{
			if (presented % 100 == 0){ fprintf(stderr, "\r%6lu%6lu/%5lu%6lu", user, number_seen, count, presented); fflush(stderr); }
			
			/*
				Get the next movie to present.
			*/
			next_movie = generator->next_movie(user, presented, key);
			
			/*
				If they can see it, do some shit.
			*/
			if ((key = (uint64_t *)bsearch(&next_movie, ratings, count, sizeof(*ratings), movie_search)))
			{
				if (stats->stats & CSP_stats::HIT_RATE)
					printf("H %lu %lu\n", user, presented + 1);
				
				if (stats->stats & CSP_stats::AUC)
					auc += ((1.0 * presented / present_max) - (1.0 * last_presented_and_seen / present_max)) * (1.0 * number_seen / count);
				number_seen++;
				
				/*
					Add the rating we came across.
				*/
				dataset->add_rating(key);
				predictor->added_rating(key);
				
				/*
					Now check our error for this user.
				*/
				if (stats->stats & CSP_stats::ERROR_RATED || stats->stats & CSP_stats::ERROR_PRESENTED)
					last_prediction_error = metric->score(user);
				
				/*
					Update the error as function of number rated.
				*/
				error_rated[number_seen] += last_prediction_error;
				
				/*
					Make a note of the last movie we saw, for AUC
				*/
				last_presented_and_seen = presented;
			}
			
			/*
				Move onto the next movie.
			*/
			presented++;
			
			/*
				Update the error as function of number presented.
			*/
			error_presented[presented] += last_prediction_error;
			
			if (stats->stats & CSP_stats::TTEST && presented % 25 == 0) 
				printf("T %lu %f\n", presented, last_prediction_error);
		}
		
		if (stats->stats & CSP_stats::FINISH)
			printf("F %lu %lu\n", user, presented);
		
		/*
			Go back and re-add ratings that we didn't find.
		*/
		for (rating = 0; rating < count; rating++)
			if (!dataset->included(ratings[rating]))
			{
				dataset->add_rating(&ratings[rating]);
				predictor->added_rating(&ratings[rating]);
			}
		
		/*
			Print out the AUC for this user for this presentation list.
		*/
		if (stats->stats & CSP_stats::AUC)
			if (last_presented_and_seen)
				printf("A %lu %f\n", user, auc + (1 - (1.0 * last_presented_and_seen / present_max)));
	
		/*
			Fill in the 'missing' values to give smooth graphs.
		*/
		for (item = number_seen + 1; item <= present_max; item++)
			error_rated[item] += last_prediction_error;
		
		for (item = presented + 1; item <= present_max; item++)
			error_presented[item] += last_prediction_error;
	}
	end = clock();
	
	fprintf(stderr, "\n");
	fprintf(stderr, "Elapsed time: %lf\n", (double)(end - start)/CLOCKS_PER_SEC);

	if (stats->stats & CSP_stats::ERROR_PRESENTED)
		for (item = 0; item <= present_max; item++)
			printf("P %lu %f\n", item, error_presented[item] / number_users);
	if (stats->stats & CSP_stats::ERROR_RATED)
		for (item = 0; item <= present_max; item++)
			printf("R %lu %f\n", item, error_rated[item] / number_users);
	
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
