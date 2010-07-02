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
	error_reduction = new movie[1];
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
	CSP_GENERATOR_GREEDY_CHEAT::GENERATE()
	--------------------------------------
*/
uint64_t *CSP_generator_greedy_cheat::generate(uint64_t user, uint64_t number_presented)
{
	uint64_t *user_ratings, user_count;
	uint64_t included = 0, i;
	
	user_ratings = dataset->ratings_for_user(user, &user_count);
	
	if (number_presented == 0)
	{
		delete [] error_reduction;
		error_reduction = new movie[user_count];
	
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
		
		for (i = 0; i < included; i++)
			presentation_list[i] = error_reduction[i].movie_id;
	}
	
	return presentation_list;
}
