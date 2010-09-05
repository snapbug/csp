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
	most_greedy = new movie[dataset->number_items];
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
	uint64_t i, j;
	
	if (which_one == 0)
	{
		/*
			Reset all the counts
		*/
		for (i = 0; i < dataset->number_items; i++)
		{
			most_greedy[i].movie_id = i;
			most_greedy[i].number_times = 0;
		}
		/*
			See what the greedy choices for everyone but this user would be.
		*/
		for (i = 0; i < dataset->number_users; i++)
			for (j = 0; j < NUMCONSIDER && i != user; j++)
				most_greedy[greedy_movies[(NUMDONE * i) + j]].number_times++;
		
		/*
			Sort by the number of times it appears.
		*/
		qsort(most_greedy, dataset->number_items, sizeof(*most_greedy), CSP_generator_other_greedy::number_times_cmp);
	}
	
	return most_greedy[which_one].movie_id;
}
