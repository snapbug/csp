/*
	GENERATOR_GREEDY_CHEAT.C
	------------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include "generator_greedy_cheat.h"

/*
	CSP_GENERATOR_GREEDY_CHEAT::CSP_GENERATOR_GREEDY_CHEAT()
	--------------------------------------------------------
*/
CSP_generator_greedy_cheat::CSP_generator_greedy_cheat(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric) : CSP_generator(dataset), metric(metric), predictor(predictor) 
{
	error_reduction = new movie[dataset->number_items];
}

/*
	CSP_GENERATOR_GREEDY_CHEAT::ERROR_CMP()
	---------------------------------------
*/
int CSP_generator_greedy_cheat::error_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	return (x->prediction_error > y->prediction_error) - (x->prediction_error < y->prediction_error);
}

/*
	CSP_GENERATOR_OTHER_GREEDY::NEXT_MOVIE()
	----------------------------------------
*/
uint64_t CSP_generator_greedy_cheat::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	UNUSED(key);
	uint64_t *user_ratings, user_count;
	uint64_t included = 0, i;
	
	user_ratings = dataset->ratings_for_user(user, &user_count);
	
	for (i = 0; i < user_count; i++)
	{
		/*
			If it hasn't been added already, add it, see what error we'd get, then remove it again.
		*/
		if (!dataset->included(user_ratings[i]))
		{
			dataset->add_rating(&user_ratings[i]);
			predictor->added_rating(&user_ratings[i]);
			
			error_reduction[included].movie_id = dataset->movie(user_ratings[i]);
			error_reduction[included].prediction_error = metric->score(user);
			
			dataset->remove_rating(&user_ratings[i]);
			predictor->removed_rating(&user_ratings[i]);
			
			included++;
		}
	}
	
	qsort(error_reduction, included, sizeof(*error_reduction), CSP_generator_greedy_cheat::error_cmp);
	
	return error_reduction[which_one].movie_id;
}
