/*
	CSP.C
	-----
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "csp_types.h"
#include "dataset_netflix.h"
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
	CSP_dataset *dataset; // TODO: REPLACE WITH FACTORY, WRITE FACTORY
	CSP_predictor *predictor;
	CSP_stats *stats;
	//CSP_metric_factory *metric = new CSP_metric_factory;
	//CSP_metric_rmse *metric = new CSP_metric_rmse;
	CSP_metric_mae *metric;
	uint64_t *presentation_list, *key, *ratings, *test_ratings;
	uint64_t position_up_to, last_presented_and_seen, number_seen;
	uint64_t count, test_count, user, item, presented, rating;
	double prediction, auc, last_prediction_error;
	double *sum_of_error;
	FILE *average_error;
	
	params->parse();
	dataset = new CSP_dataset_netflix(params);
	stats = new CSP_stats(params->stats);
	
	average_error = NULL;
	
	sum_of_error = new double[dataset->number_items];
	for (item = 0; item < dataset->number_items; item++)
		sum_of_error[item] = 0;
	
	switch (params->generation_method)
	{
		case CSP_generator_factory::BAYESIAN: generator = new CSP_generator_naive_bayes(dataset); break;
		case CSP_generator_factory::ENTROPY: generator = new CSP_generator_entropy(dataset); break;
		case CSP_generator_factory::ITEM_AVERAGE: generator = new CSP_generator_item_avg(dataset); break;
		case CSP_generator_factory::POPULARITY: generator = new CSP_generator_popularity(dataset); break;
		case CSP_generator_factory::RANDOM: generator = new CSP_generator_random(dataset); break;
		default: exit(puts("Unknown generation method"));
	}
	
	switch (params->prediction_method)
	{
		case CSP_predictor_factory::CONSTANT: predictor = new CSP_predictor_constant(dataset); break;
		case CSP_predictor_factory::GLOBAL_AVERAGE: predictor = new CSP_predictor_global_avg(dataset); break;
		case CSP_predictor_factory::ITEM_AVERAGE: predictor = new CSP_predictor_item_avg(dataset); break;
		case CSP_predictor_factory::ITEM_ITEM_KNN: predictor = new CSP_predictor_item_knn(dataset, 20); break;
		case CSP_predictor_factory::KORBELL: predictor = new CSP_predictor_korbell(dataset, 25.0); break;
		case CSP_predictor_factory::RANDOM: predictor = new CSP_predictor_random(dataset); break;
		case CSP_predictor_factory::USER_AVERAGE: predictor = new CSP_predictor_user_avg(dataset); break;
		case CSP_predictor_factory::USER_USER_KNN: predictor = new CSP_predictor_user_knn(dataset, 20); break;
		default: exit(puts("Unknown prediction method"));
	}
	
	metric = new CSP_metric_mae(dataset, predictor);
	//metric->set_metrics(params->metrics_to_use);
	//metric->set_limits(dataset->maximum, dataset->minimum);
	
	/*
		For each user we're simulating a coldstart for. (Initial testee = 168)
	*/
	for (user = 0; user < dataset->number_users; user++)
	//if (false)
	{
		if (user % 1000 == 0) { fprintf(stderr, "\r%lu", user); fflush(stderr); }
		
		/*
			Reset things for this user.
		*/
		position_up_to = last_presented_and_seen = number_seen = 0;
		presentation_list = key = NULL;
		auc = last_prediction_error = 0;
		
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
		last_prediction_error = metric->score(user);
		
		/*
			While the user can still add more ratings.
		*/
		while (number_seen < count)
		{
			/*
				Generate the list of movies to present to the user.
			*/
			presentation_list = generator->generate(user, position_up_to);
			
			/*
				Find the next rating that the person can rate.
			*/
			for (presented = position_up_to; presented < dataset->number_items; presented++)
			{
				sum_of_error[presented] += last_prediction_error;
				
				if ((key = (uint64_t *)bsearch(&presentation_list[presented], ratings, count, sizeof(*ratings), movie_search)) != NULL)
				{
					/*
						Update the area under the curve up to this point.
					*/
					auc += ((1.0 * presented / dataset->number_items) - (1.0 * last_presented_and_seen / dataset->number_items)) * (1.0 * number_seen++ / count);
					
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
					last_prediction_error = metric->score(user);
					
					/*
						Stop looking for the next rating so we can check our accuracy.
					*/
					break;
				}
			}
		}
		
		/*
			Need to keep going with the users with all the potential they can add, so as not to screw up averages.
		*/
		for(presented = position_up_to; presented < dataset->number_items; presented++)
			sum_of_error[presented] += last_prediction_error;
		
		/*
			Update the AUC for the presentation list, and print it out.
		*/
		auc += (1 - (1.0 * last_presented_and_seen / dataset->number_items));
		//printf("AUC\t%lu\t%f\n", user, auc);
	}

	/*
		Print our average errors to file.
	*/
//	average_error = fopen("pk(gmuas).gi.txt", "w");
	for (item = 0; item < dataset->number_items; item++)
		fprintf(stdout, "%lu %f\n", item, sum_of_error[item] / dataset->number_users);
//	fclose(average_error);
	
//	for (user = 0; user < dataset->number_users; user++)
//	{
//		metric->reset();
//		test_ratings = dataset->test_ratings_for_user(user, &test_count);
//		for (rating = 0; rating < test_count; rating++)
//		{
//			prediction = predictor->predict(user, dataset->movie(test_ratings[rating]), dataset->day(test_ratings[rating]));
//			prediction = clip(prediction, dataset->minimum, dataset->maximum);
//			metric->update(prediction, (double)dataset->rating(test_ratings[rating]));
//		}
//		sum_of_error[0] += metric->score();
//	}
//	printf("%f\n", sum_of_error[0] / dataset->number_users);
	
	/*
		Clean up.
	*/
	delete params;
	delete generator;
	delete dataset;
	delete predictor;
	delete metric;
	
	return EXIT_SUCCESS;
}
