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
	CSP_GENERATOR_OTHER_GREEDY_PERS::CALCULATE_PROBABILITY()
	--------------------------------------------------------
*/
double CSP_generator_other_greedy_pers::calculate_probability(uint64_t movie, uint64_t other, uint64_t *key)
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
	CSP_GENERATOR_OTHER_GREEDY_PERS::NEXT_MOVIE()
	---------------------------------------------
*/
uint64_t CSP_generator_other_greedy_pers::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	UNUSED(key);
	uint64_t i, count, next;
	uint64_t *user_ratings;
	
	if (which_one == 0)
	{
		user_ratings = dataset->ratings_for_user(user, &count);
		
		/*
			Reset the counts for the number of times we counted it
		*/
		for (i = 0; i < dataset->number_items; i++)
		{
			dataset->ratings_for_movie(i, &count);
			number_times_greedy[i].movie_id = i;
			number_times_greedy[i].number_times = number_times_start[i];//1.0 * number_times_start[i] / dataset->number_users;
			number_times_greedy[i].probability = log(1.0 * count / (dataset->number_users - count));
		}
		
		for (i = 0; i < NUMCONSIDER; i++)
		{
			/*
				See what the top would have been for this user in this position.
			*/
			next = CSP_generator_greedy_cheat::next_movie(user, i, key);
		
			/*
				Remove the count so we can resort properly.
			*/
			number_times_greedy[next].number_times--;
		}
		
		/*
			Re-remove all the ratings again.
		*/
		for (i = 0; i < count; i++)
		{
			if (dataset->included(user_ratings[i]))
			{
				dataset->remove_rating(&user_ratings[i]);
				predictor->removed_rating(&user_ratings[i]);
			}
		}
	}
	else
	{
		/*
			For each remaining item, need to update the probabilities we've seen them.
		*/
		for (i = which_one; i < dataset->number_items; i++)
			number_times_greedy[i].probability += calculate_probability(number_times_greedy[i].movie_id, number_times_greedy[which_one - 1].movie_id, key);
	}
	
	qsort(number_times_greedy + which_one, dataset->number_items - which_one, sizeof(*number_times_greedy), CSP_generator_other_greedy_pers::prob_times_cmp);
	
	return number_times_greedy[which_one].movie_id;
}
