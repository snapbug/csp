/*
	GENERATOR_NAIVE_BAYES.C
	-----------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "generator_naive_bayes.h"

/*
	CSP_GENERATOR_NAIVE_BAYES::CSP_GENERATOR_NAIVE_BAYES()
	------------------------------------------------------
*/
CSP_generator_naive_bayes::CSP_generator_naive_bayes(CSP_dataset *dataset, uint32_t *coraters) : CSP_generator_entropy(dataset), coraters(coraters)
{
	if (!dataset->loaded_extra)
		exit(puts("Need to load data sorted by movie (-e) to use Naive Bayes"));
	
	movies = new movie[dataset->number_items];
}

/*
	CSP_GENERATOR_NAIVE_BAYES::PROBABILITY_CMP()
	--------------------------------------------
*/
int CSP_generator_naive_bayes::probability_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	double x_ratio = x->probability_yes / x->probability_no;
	double y_ratio = y->probability_yes / y->probability_no;
	
	return (x_ratio > y_ratio) - (x_ratio < y_ratio);
}

/*
	CSP_GENERATOR_NAIVE_BAYES::CALCULATE_PROBABILITY_YES()
	------------------------------------------------------
*/
double CSP_generator_naive_bayes::calculate_probability_yes(uint64_t movie, uint64_t non_ratable, uint64_t ratable)
{
	double probability = 0;
	uint64_t min, max, count, other_count;
#ifdef NON_RATABLE
	uint64_t i = non_ratable;
#else
	UNUSED(non_ratable);
#endif
	
	dataset->ratings_for_movie(movie, &count);
	
#ifdef NON_RATABLE
	for (; i < ratable; i++)
	{
		dataset->ratings_for_movie(presentation_list[i], &other_count);
		min = MIN(movie, presentation_list[i]);
		max = MAX(movie, presentation_list[i]);
		probability += log((1.0 * count + 1 - coraters[tri_offset(min, max)]) / (1 + dataset->number_users - other_count));
	}
#endif
	
	dataset->ratings_for_movie(presentation_list[ratable], &other_count);
	min = MIN(movie, presentation_list[ratable]);
	max = MAX(movie, presentation_list[ratable]);
	return probability + log((1.0 * coraters[tri_offset(min, max)] + 1) / (other_count + 1));
}

/*
	CSP_GENERATOR_NAIVE_BAYES::CALCULATE_PROBABILITY_NO()
	-----------------------------------------------------
*/
double CSP_generator_naive_bayes::calculate_probability_no(uint64_t movie, uint64_t non_ratable, uint64_t ratable)
{
	double probability = 0;
	uint64_t min, max, count, other_count;
#ifdef NON_RATABLE
	uint64_t i = non_ratable;
#else
	UNUSED(non_ratable);
#endif
	
	dataset->ratings_for_movie(movie, &count);
	
#ifdef NON_RATABLE
	for (; i < ratable; i++)
	{
		dataset->ratings_for_movie(presentation_list[i], &other_count);
		min = MIN(movie, presentation_list[i]);
		max = MAX(movie, presentation_list[i]);
		probability += log((1.0 * dataset->number_users - other_count - count + coraters[tri_offset(min, max)]) / (dataset->number_users - other_count));
	}
#endif
	
	dataset->ratings_for_movie(presentation_list[ratable], &other_count);
	min = MIN(movie, presentation_list[ratable]);
	max = MAX(movie, presentation_list[ratable]);
	return probability + log((1.0 * other_count - coraters[tri_offset(min, max)]) / (other_count));
}

/*
	CSP_GENERATOR_NAIVE_BAYES::GENERATE()
	-------------------------------------
*/
uint64_t *CSP_generator_naive_bayes::generate(uint64_t user, uint64_t number_presented)
{
	uint64_t i;
	
	if (number_presented == 0)
	{
		CSP_generator_entropy::generate(user, number_presented);
		
		for (i = 0; i < dataset->number_items; i++)
		{
			movies[i].movie_id = presentation_list[i];
			movies[i].probability_yes = 0;
			movies[i].probability_no = 0;
		}
	}
	else
	{
		/*
			For each remaining item, need to update the probabilities we've seen them.
		*/
		for (i = number_presented; i < dataset->number_items; i++)
		{
			movies[i].probability_yes += calculate_probability_yes(movies[i].movie_id, last_presented_and_seen, number_presented - 1);
			movies[i].probability_no += calculate_probability_no(movies[i].movie_id, last_presented_and_seen, number_presented - 1);
		}
		
		qsort(movies + number_presented, dataset->number_items - number_presented, sizeof(*movies), CSP_generator_naive_bayes::probability_cmp);
		
		for (i = number_presented; i < dataset->number_items; i++)
			presentation_list[i] = movies[i].movie_id;
	}
	last_presented_and_seen = number_presented;
	
	return presentation_list;
}
