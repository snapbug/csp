/*
	GENERATOR_OTHER_GREEDY.C
	------------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "generator_other_greedy.h"

/*
	CSP_GENERATOR_OTHER_GREEDY::CSP_GENERATOR_OTHER_GREEDY()
	--------------------------------------------------------
*/
CSP_generator_other_greedy::CSP_generator_other_greedy(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric) : CSP_generator_greedy_cheat(dataset, predictor, metric)
{
	number_times_greedy = new movie[dataset->number_items];
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
	CSP_GENERATOR_OTHER_GREEDY::MOVIE_ID_SEARCH()
	---------------------------------------------
*/
int CSP_generator_other_greedy::movie_id_search(const void *a, const void *b)
{
	uint64_t key = *(uint64_t *)a;
	uint64_t item = (*(uint64_t *)b) >> 15 & 32767;
	return (key > item) - (key < item);
}

/*
	CSP_GENERATOR_OTHER_GREEDY::NEXT_MOVIE()
	----------------------------------------
*/
uint64_t CSP_generator_other_greedy::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	UNUSED(key);
	uint64_t i, count, mov;
	uint64_t *user_ratings, *rating;
	
	if (which_one == 0)
	{
		user_ratings = dataset->ratings_for_user(user, &count);
		
		/*
			Reset the counts for the number of times we counted it
		*/
		for (i = 0; i < dataset->number_items; i++)
		{
			number_times_greedy[i].movie_id = i;
			number_times_greedy[i].number_times = number_times_start[i];
		}
		
		for (i = 0; i < NUMCONSIDER; i++)
		{
			/*
				See what the top would have been for this user in this position.
				Remove the count so we can resort properly.
			*/
			mov = CSP_generator_greedy_cheat::next_movie(user, i, key);
			number_times_greedy[mov].number_times--;
			
			/*
				Add it in so that the greedy generator can get the correct result for the next position
			*/
			rating = (uint64_t *)bsearch(&mov, user_ratings, count, sizeof(*user_ratings), movie_id_search);
			dataset->add_rating(rating);
			predictor->added_rating(rating);
		}
		
		/*
			Remove all the ratings again so we can get the measure right
		*/
		for (i = 0; i < count; i++)
		{
			if (dataset->included(user_ratings[i]))
			{
				dataset->remove_rating(&user_ratings[i]);
				predictor->removed_rating(&user_ratings[i]);
			}
		}
		
		/*
			Sort by the number of times it appears.
		*/
		qsort(number_times_greedy, dataset->number_items, sizeof(*number_times_greedy), CSP_generator_other_greedy::number_times_cmp);
	}
	
	return number_times_greedy[which_one].movie_id;
}
