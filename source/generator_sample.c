/*
	GENERATOR_SAMPLE.C
	------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "generator_sample.h"

/*
	CSP_GENERATOR_SAMPLE::CSP_GENERATOR_SAMPLE()
	--------------------------------------------
*/
CSP_generator_sample::CSP_generator_sample(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric) : CSP_generator(dataset), metric(metric), predictor(predictor) 
{
	error_reduction = new error_movie[dataset->number_items];
	first_positions = new count_movie[dataset->number_items];
}

/*
	CSP_GENERATOR_SAMPLE::ERROR_CMP()
	---------------------------------
*/
int CSP_generator_sample::error_cmp(const void *a, const void *b)
{
	error_movie *x = (error_movie *)a;
	error_movie *y = (error_movie *)b;
	
	return (x->prediction_error > y->prediction_error) - (x->prediction_error < y->prediction_error);
}

/*
	CSP_GENERATOR_SAMPLE::COUNT_CMP()
	---------------------------------
*/
int CSP_generator_sample::count_cmp(const void *a, const void *b)
{
	count_movie *x = (count_movie *)a;
	count_movie *y = (count_movie *)b;
	
	return (x->count < y->count) - (x->count > y->count);
}

/*
	CSP_GENERATOR_SAMPLE::GENERATE()
	--------------------------------
*/
uint64_t *CSP_generator_sample::generate(uint64_t user, uint64_t number_presented)
{
	uint64_t *user_ratings, user_count;
	uint64_t sample_user = user;
	uint64_t i, j;
	
	/*
		If we have a new user, reset the error_reduction array.
		Also do a new sample to work out what order to present movies.
	*/
	if (number_presented == 0)
	{
		for (i = 0; i < dataset->number_items; i++)
		{
			first_positions[i].movie_id = i;
			first_positions[i].count = 0;
		}
		
		for (i = 0; i < 20000; i++)
		{
			sample_user = (uint64_t)(rand() / (RAND_MAX / (double)(dataset->number_users) + 1));
			while (sample_user == user)
				sample_user = (uint64_t)(rand() / (RAND_MAX / (double)(dataset->number_users) + 1));
			
			user_ratings = dataset->ratings_for_user(sample_user, &user_count);
			
			for (j = 0; j < user_count; j++)
			{
				dataset->remove_rating(&user_ratings[j]);
				predictor->removed_rating(&user_ratings[j]);
			}
			
			for (j = 0; j < user_count; j++)
			{
				if (j % 100 == 0) { fprintf(stderr, "\r%6lu: [%5lu] %6lu (%5lu/%5lu)", user, i, sample_user, j, user_count); fflush(stderr); }
				dataset->add_rating(&user_ratings[j]);
				predictor->added_rating(&user_ratings[j]);
				
				error_reduction[j].movie_id = dataset->movie(user_ratings[j]);
				error_reduction[j].prediction_error = metric->score(sample_user);
				
				dataset->remove_rating(&user_ratings[j]);
				predictor->removed_rating(&user_ratings[j]);
			}
			
			for (j = 0; j < user_count; j++)
			{
				dataset->add_rating(&user_ratings[j]);
				predictor->added_rating(&user_ratings[j]);
			}
			
			qsort(error_reduction, user_count, sizeof(*error_reduction), CSP_generator_sample::error_cmp);
			
			j = 0;
			for (j = 0; j < MIN(5, user_count); j++)
				first_positions[error_reduction[j].movie_id].count++;
		}
		qsort(first_positions, dataset->number_items, sizeof(*first_positions), CSP_generator_sample::count_cmp);
		
		for (i = 0; i < dataset->number_items; i++)
			presentation_list[i] = first_positions[i].movie_id;
	}
	
	return presentation_list;
}
