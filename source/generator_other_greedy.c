/*
	GENERATOR_OTHER_GREEDY.C
	------------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include "generator_other_greedy.h"
#define NUMCONSIDER 5

static uint64_t number_times_start[] =
#include "init.greedy.dat"

/*
	CSP_GENERATOR_OTHER_GREEDY::CSP_GENERATOR_OTHER_GREEDY()
	--------------------------------------------------------
*/
CSP_generator_other_greedy::CSP_generator_other_greedy(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric) : CSP_generator_greedy_cheat(dataset, predictor, metric), metric(metric), predictor(predictor)
{
	uint64_t i;
	
	number_times_greedy = new movie[dataset->number_items];
	ones_changed = new uint64_t[NUMCONSIDER];
	
	for (i = 0; i < NUMCONSIDER; i++)
		ones_changed[i] = dataset->number_items;
	
	for (i = 0; i < dataset->number_items; i++)
	{
		number_times_greedy[i].movie_id = i;
		number_times_greedy[i].number_times = number_times_start[i];
	}
}

/*
	CSP_GENERATOR_OTHER_GREEDY::MOVIE_ID_CMP()
	------------------------------------------
*/
int CSP_generator_other_greedy::movie_id_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	return (x->movie_id > y->movie_id) - (x->movie_id < y->movie_id);
}

/*
	CSP_GENERATOR_OTHER_GREEDY::NUMBER_TIMES_CMP()
	----------------------------------------------
*/
int CSP_generator_other_greedy::number_times_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	return (x->number_times < y->number_times) - (x->number_times > y->number_times);
}

/*
	CSP_GENERATOR_OTHER_GREEDY::GENERATE()
	--------------------------------------
*/
uint64_t *CSP_generator_other_greedy::generate(uint64_t user, uint64_t number_presented)
{
	uint64_t i, count;
	uint64_t *user_ratings;
	
	UNUSED(user);
	
	if (number_presented == 0)
	{
		user_ratings = dataset->ratings_for_user(user, &count);
		
		qsort(number_times_greedy, dataset->number_items, sizeof(*number_times_greedy), CSP_generator_other_greedy::movie_id_cmp);
		
		/*
			Reset the counts for the number of times we counted it
		*/
		for (i = 0; i < dataset->number_items; i++)
			number_times_greedy[i].number_times = number_times_start[number_times_greedy[i].movie_id];
		
		for (i = 0; i < NUMCONSIDER; i++)
		{
			/*
				See what the top would have been for this user in this position.
			*/
			CSP_generator_greedy_cheat::generate(user, i);
		
			/*
				Remove the count so we can resort properly.
			*/
			number_times_greedy[presentation_list[i]].number_times--;
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
		
		qsort(number_times_greedy, dataset->number_items, sizeof(*number_times_greedy), CSP_generator_other_greedy::number_times_cmp);
		
		for (i = 0; i < dataset->number_items; i++)
			presentation_list[i] = number_times_greedy[i].movie_id;
	}
	
	return presentation_list;
}
