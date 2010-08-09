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
	
	most_probable = new movie[dataset->number_items];
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
double CSP_generator_naive_bayes::calculate_probability(uint64_t movie, uint64_t other, uint64_t *key)
{
	double probability = 0, prob_yes, prob_no;
	uint64_t min, max, count, other_count;
	
	dataset->ratings_for_movie(movie, &count);
	
	if (key != NULL) // means it was found in user list
	{
		dataset->ratings_for_movie(other, &other_count);
		min = MIN(movie, other);
		max = MAX(movie, other);
		
		prob_yes = (1.0 * coraters[tri_offset(min, max)] + 1) / (other_count + 1);
		prob_no = (1.0 * other_count - coraters[tri_offset(min, max)]) / other_count;
		
		probability += log(prob_yes / prob_no);
	}
	else
	{
#ifdef NON_RATABLE
		dataset->ratings_for_movie(other, &other_count);
		min = MIN(movie, other);
		max = MAX(movie, other);
		
		prob_yes = (1.0 * count + 1 - coraters[tri_offset(min, max)]) / (1 + dataset->number_users - other_count);
		prob_no = (1.0 * dataset->number_users - other_count - count + coraters[tri_offset(min, max)]) / (dataset->number_users - other_count);
		
		probability += log(prob_yes / prob_no);
#endif
	}
	
	return probability;
}


/*
	CSP_GENERATOR_NAIVE_BAYES::NEXT_MOVIE()
	---------------------------------------
*/
uint64_t CSP_generator_naive_bayes::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	UNUSED(user);
	uint64_t i, count;
	
	if (which_one == 0)
	{
		for (i = 0; i < dataset->number_items; i++)
		{
			dataset->ratings_for_movie(i, &count);
			most_probable[i].movie_id = i;
			most_probable[i].probability = log(1.0 * count / (dataset->number_users - count));
		}
	}
	else
	{
		/*
			For each remaining item, need to update the probabilities we've seen them.
		*/
		for (i = which_one; i < dataset->number_items; i++)
		{
			most_probable[i].probability += calculate_probability(most_probable[i].movie_id, most_probable[which_one - 1].movie_id, key);
		}
	}
	
	qsort(most_probable + which_one, dataset->number_items - which_one, sizeof(*most_probable), CSP_generator_naive_bayes::probability_cmp);
	
	return most_probable[which_one].movie_id;
}
