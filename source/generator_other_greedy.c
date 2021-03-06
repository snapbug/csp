/*
	GENERATOR_OTHER_GREEDY.C
	------------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "generator_other_greedy.h"

//#define SWITCHING

/*
	CSP_GENERATOR_OTHER_GREEDY::CSP_GENERATOR_OTHER_GREEDY()
	--------------------------------------------------------
*/
CSP_generator_other_greedy::CSP_generator_other_greedy(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric) : CSP_generator_greedy_cheat(dataset, predictor, metric)
{
	uint64_t i, j;
	
	most_greedy = new movie[dataset->number_items];
	
	for (i = 0; i < dataset->number_items; i++)
		most_greedy[i].movie_id = i;
	for (i = 0; i < dataset->number_users; i++)
		for (j = 0; j < NUMCONSIDER; j++)
			most_greedy[greedy_movies[(NUMDONE * i) + j]].number_times++;
	qsort(most_greedy, dataset->number_items, sizeof(*most_greedy), CSP_generator_other_greedy::number_times_cmp);
	for (i = 0; i < 20; i++)
		printf("%lu\n", most_greedy[i].movie_id);
	exit(EXIT_SUCCESS);
}

/*
	CSP_GENERATOR_OTHER_GREEDY::NUMBER_TIMES_CMP()
	----------------------------------------------
*/
int CSP_generator_other_greedy::number_times_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	if (x->included && !y->included) return -1;
	if (!x->included && y->included) return 1;
	return (x->number_times < y->number_times) - (x->number_times > y->number_times);
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
	CSP_GENERATOR_OTHER_GREEDY::NEXT_MOVIE()
	----------------------------------------
*/
uint64_t CSP_generator_other_greedy::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	UNUSED(key);
	uint64_t i, j;
	
#ifdef SWITCHING
	NUMCONSIDER = which_one >= 15 ? 1 : 5;
#endif
	
	if (which_one == 0)
	{
		/*
			Reset invariants
		*/
		for (i = 0; i < dataset->number_items; i++)
		{
			most_greedy[i].movie_id = i;
			most_greedy[i].included = FALSE;
		}
#ifdef SWITCHING
	}
#endif
	
	/*
		Reset counts
	*/
	for (i = 0; i < dataset->number_items; i++)
		most_greedy[i].number_times = 0;
	
#ifdef SWITCHING
	qsort(most_greedy, dataset->number_items, sizeof(*most_greedy), CSP_generator_other_greedy::movie_id_cmp);
#endif
	
	/*
		See what the greedy choices for everyone but this user would be.
	*/
	for (i = 0; i < dataset->number_users; i++)
		for (j = 0; j < NUMCONSIDER && i != user; j++)
			if (greedy_movies[(NUMDONE * i) + j] < dataset->number_items)
				most_greedy[greedy_movies[(NUMDONE * i) + j]].number_times++;
	
	/*
		Sort by the number of times it appears.
	*/
	qsort(most_greedy, dataset->number_items, sizeof(*most_greedy), CSP_generator_other_greedy::number_times_cmp);
	
#ifdef SWITCHING
	most_greedy[which_one].included = TRUE;
#else
	}
#endif
	return most_greedy[which_one].movie_id;
}
