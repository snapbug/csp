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
CSP_generator_other_greedy_pers::CSP_generator_other_greedy_pers(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric, uint32_t *coraters) : CSP_generator_other_greedy(dataset, predictor, metric), metric(metric), predictor(predictor), coraters(coraters)
{
	uint64_t i;
	
	number_times_greedy = new movie[dataset->number_items];
	ones_changed = new uint64_t[NUMCONSIDER];
	
	for (i = 0; i < NUMCONSIDER; i++)
		ones_changed[i] = dataset->number_items;
	
}

/*
	CSP_GENERATOR_OTHER_GREEDY_PERS::PROB_TIMES_CMP()
	-------------------------------------------------
*/
int CSP_generator_other_greedy_pers::prob_times_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	double wx = (x->number_times / 429584.0) * x->probability;
	double wy = (y->number_times / 429584.0) * y->probability;
	
	return (wx > wy) - (wx < wy);
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
	
	return (x->probability > y->probability) - (x->probability < y->probability);
}

/*
	CSP_GENERATOR_OTHER_GREEDY_PERS::CALCULATE_PROBABILITY()
	--------------------------------------------------------
*/
double CSP_generator_other_greedy_pers::calculate_probability(uint64_t movie, uint64_t other, uint64_t *key)
{
	double prob_yes = 1, prob_no = 1;
	uint64_t min, max, count, other_count;
	
	dataset->ratings_for_movie(movie, &count);
	
	if (key != NULL) // means it was found in user list
	{
		dataset->ratings_for_movie(other, &other_count);
		min = MIN(movie, other);
		max = MAX(movie, other);
		
		prob_yes = (1.0 * coraters[tri_offset(min, max)] + 1) / (other_count + 1);
		prob_no = (1.0 * other_count - coraters[tri_offset(min, max)]) / other_count;
	}
	else
	{
#ifdef NON_RATABLE
		dataset->ratings_for_movie(other, &other_count);
		min = MIN(movie, other);
		max = MAX(movie, other);
		
		prob_yes = (1.0 * count + 1 - coraters[tri_offset(min, max)]) / (1 + dataset->number_users - other_count);
		prob_no = (1.0 * dataset->number_users - other_count - count + coraters[tri_offset(min, max)]) / (dataset->number_users - other_count);
#endif
	}
	
	return log(prob_yes / prob_no);
}

/*
	CSP_GENERATOR_OTHER_GREEDY_PERS::NEXT_MOVIE()
	---------------------------------------------
*/
uint64_t CSP_generator_other_greedy_pers::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	UNUSED(key);
	uint64_t i, count;
	int64_t index;
	
	if (which_one == 0)
	{
		/*
			Reset the information for each movie.
		*/
		for (i = 0; i < dataset->number_items; i++)
		{
			dataset->ratings_for_movie(i, &count);
			number_times_greedy[i].movie_id = i;
			number_times_greedy[i].number_times = (double)number_times_start[i];
			number_times_greedy[i].probability = log(1.0 * count / (dataset->number_users - count));
		}
		
		/*
			For each position we are considering, see what would have been in that position,
			then remove the count so it re-sorts correctly.
		*/
		for (i = 0; i < NUMCONSIDER; i++)
			number_times_greedy[CSP_generator_greedy_cheat::next_movie(user, i, key)].number_times--;
	}
	else
	{
		/*
			For each remaining item, need to update the probabilities we've seen them.
		*/
		#pragma omp parallel for
		for (index = (int64_t)which_one; index < (int64_t)dataset->number_items; index++)
			number_times_greedy[index].probability += calculate_probability(number_times_greedy[index].movie_id, number_times_greedy[which_one - 1].movie_id, key);
	}
	
	if (key)
	{
		/*
			If they could see the last one we gave them, keep going with the list we already worked out.
			---
			Probably want something here depending on the rating they gave?
		*/
		qsort(number_times_greedy + which_one, dataset->number_items - which_one, sizeof(*number_times_greedy), CSP_generator_other_greedy_pers::number_times_cmp);
	}
	else
	{
		/*
			Otherwise, we want to see if we can get a comparably information content movie,
			that they are more likely to rate.
		*/
		qsort(number_times_greedy + which_one, (which_one + NUMCONSIDER < dataset->number_items ? NUMCONSIDER : dataset->number_items - NUMCONSIDER), sizeof(*number_times_greedy), CSP_generator_other_greedy_pers::probability_cmp);
	}
	
	return number_times_greedy[which_one].movie_id;
}
