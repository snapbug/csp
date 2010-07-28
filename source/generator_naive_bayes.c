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
	
	return (x->probability < y->probability) - (x->probability > y->probability);
}

/*
	CSP_GENERATOR_NAIVE_BAYES::CALCULATE_PROBABILITY()
	--------------------------------------------------
*/
double CSP_generator_naive_bayes::calculate_probability(uint64_t movie, uint64_t non_ratable, uint64_t ratable)
{
	double probability = 0, prob_yes, prob_no;
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
		prob_yes = (1.0 * count + 1 - coraters[tri_offset(min, max)]) / (1 + dataset->number_users - other_count);
		prob_no = (1.0 * dataset->number_users - other_count - count + coraters[tri_offset(min, max)]) / (dataset->number_users - other_count);
		probability += log(prob_yes / prob_no);
	}
#endif
	
	dataset->ratings_for_movie(presentation_list[ratable], &other_count);
	min = MIN(movie, presentation_list[ratable]);
	max = MAX(movie, presentation_list[ratable]);
	prob_yes = (1.0 * coraters[tri_offset(min, max)] + 1) / (other_count + 1);
	prob_no = (1.0 * other_count - coraters[tri_offset(min, max)]) / other_count;
	
	return probability + log(prob_yes / prob_no);
}

/*
	CSP_GENERATOR_NAIVE_BAYES::GENERATE()
	-------------------------------------
*/
uint64_t *CSP_generator_naive_bayes::generate(uint64_t user, uint64_t number_presented)
{
	UNUSED(user);
	uint64_t i, count;
	
	if (number_presented == 0)
	{
		for (i = 0; i < dataset->number_items; i++)
		{
			dataset->ratings_for_movie(presentation_list[i], &count);
			movies[i].movie_id = presentation_list[i];
			movies[i].probability = log(1.0 * count / (dataset->number_users - count));
		}
	}
	else
	{
		/*
			For each remaining item, need to update the probabilities we've seen them.
			Some ugliness, as if we've seen the very first item, then ratable needs to = 0, not wrap around.
		*/
		for (i = number_presented; i < dataset->number_items; i++)
			movies[i].probability += calculate_probability(movies[i].movie_id, last_presented_and_seen, number_presented - (number_presented != 0));
	}
	last_presented_and_seen = number_presented;
	
	qsort(movies + number_presented, dataset->number_items - number_presented, sizeof(*movies), CSP_generator_naive_bayes::probability_cmp);
	
	for (i = number_presented; i < dataset->number_items; i++)
		presentation_list[i] = movies[i].movie_id;
	
	return presentation_list;
}
