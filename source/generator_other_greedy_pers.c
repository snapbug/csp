/*
	GENERATOR_OTHER_GREEDY_PERS.C
	------------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "generator_other_greedy_pers.h"

/*
	CSP_GENERATOR_OTHER_GREEDY_PERS::CSP_GENERATOR_OTHER_GREEDY_PERS()
	------------------------------------------------------------------
*/
CSP_generator_other_greedy_pers::CSP_generator_other_greedy_pers(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric, uint32_t *coraters) : CSP_generator_other_greedy(dataset, predictor, metric), coraters(coraters)
{
	most_greedy_prob = new movie[dataset->number_items];
}

/*
	CSP_GENERATOR_OTHER_GREEDY_PERS::NUMBER_TIMES_CMP()
	---------------------------------------------------
*/
int CSP_generator_other_greedy_pers::number_times_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	return (x->number_times < y->number_times) - (x->number_times > y->number_times);
}

/*
	CSP_GENERATOR_OTHER_GREEDY_PERS::PROBABILITY_CMP()
	--------------------------------------------------
*/
int CSP_generator_other_greedy_pers::probability_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	double prob_x = x->top / (x->top + x->bot);
	double prob_y = y->top / (y->top + y->bot);
	
#ifndef ASC
	return (prob_x > prob_y) - (prob_x < prob_y);
#else
	return (prob_x < prob_y) - (prob_x > prob_y);
#endif
}

/*
	CSP_GENERATOR_OTHER_GREEDY_PERS::CALCULATE_PROBABILITY()
	--------------------------------------------------------
*/
double CSP_generator_other_greedy_pers::calculate_probability(uint64_t movie, uint64_t other, uint64_t *key)
{
	uint64_t count, other_count;
	
	dataset->ratings_for_movie(movie, &count);
	dataset->ratings_for_movie(other, &other_count);
	
	if (key)
		return (1.0 * coraters[tri_offset(MIN(movie, other), MAX(movie, other), dataset->number_items)] + 1.0) / (count + 1.0);
	else
		return (1.0 * other_count - coraters[tri_offset(MIN(movie, other), MAX(movie, other), dataset->number_items)] + 1.0) / (1.0 + dataset->number_users - count);
}


/*
	CSP_GENERATOR_OTHER_GREEDY_PERS::NEXT_MOVIE()
	---------------------------------------------
*/
uint64_t CSP_generator_other_greedy_pers::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	uint64_t i, j;
	double probability;
	
	if (which_one == 0)
	{
		/*
			Reset the information for each movie.
		*/
		for (i = 0; i < dataset->number_items; i++)
		{
			most_greedy_prob[i].movie_id = i;
			most_greedy_prob[i].number_times = 0;
			most_greedy_prob[i].top = 1e300;
			most_greedy_prob[i].bot = 1e300;
		}
		
		for (i = 0; i < dataset->number_users; i++)
			for (j = 0; j < NUMCONSIDER && i != user; j++)
				most_greedy_prob[greedy_movies[(NUMDONE * i) + j]].number_times++;
		
		qsort(most_greedy_prob, dataset->number_items, sizeof(*most_greedy_prob), CSP_generator_other_greedy_pers::number_times_cmp);
	}
	else
	{
		/*
			For each remaining item, need to update the probabilities we've seen them.
		*/
		for (i = which_one; i < dataset->number_items; i++)
		{
			probability = calculate_probability(most_greedy_prob[i].movie_id, most_greedy_prob[which_one - 1].movie_id, key);
			most_greedy_prob[i].top *= probability;
			most_greedy_prob[i].bot *= 1 - probability;
		}
		
		qsort(most_greedy_prob + which_one, MIN(PERTURB, dataset->number_items - which_one), sizeof(*most_greedy_prob), CSP_generator_other_greedy_pers::probability_cmp);
	}
	
	return most_greedy_prob[which_one].movie_id;
}
