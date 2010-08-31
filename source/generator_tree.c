/*
	GENERATOR_TREE.C
	----------------
*/
#include <stdio.h>
#include <stdlib.h>
#include "generator_tree.h"

/*
	CSP_GENERATOR_TREE::CSP_GENERATOR_TREE()
	----------------------------------------
*/
CSP_generator_tree::CSP_generator_tree(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric) : CSP_generator_greedy_cheat(dataset, predictor, metric)
{
	most_greedy = new movie[dataset->number_items];
	users = new uint64_t[dataset->number_users];
}

/*
	CSP_GENERATOR_TREE::NUMBER_TIMES_CMP()
	--------------------------------------
*/
int CSP_generator_tree::number_times_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	return (x->number_times < y->number_times) - (x->number_times > y->number_times);
}

/*
	CSP_GENERATOR_TREE::MOVIE_ID_CMP()
	----------------------------------
*/
int CSP_generator_tree::movie_id_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	return (x->movie_id > y->movie_id) - (x->movie_id < y->movie_id);
}

/*
	CSP_GENERATOR_TREE::MOVIE_SEARCH()
	----------------------------------
*/
int CSP_generator_tree::movie_search(const void *a, const void *b)
{
	uint64_t key = *(uint64_t *)a;
	uint64_t item = ((movie *)b)->movie_id;
	return (key > item) - (key < item);
}

/*
	CSP_GENERATOR_TREE::NEXT_MOVIE()
	--------------------------------
*/
uint64_t CSP_generator_tree::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	uint64_t i, movie, other_user, count;
	uint64_t *user_ratings, *result;
	
	if (which_one == 0)
	{
		for (i = 0; i < dataset->number_items; i++)
		{
			most_greedy[i].movie_id = i;
			most_greedy[i].number_times = number_times[i];
		}
		
		for (i = 0; i < NUMCONSIDER; i++)
			most_greedy[CSP_generator_greedy_cheat::next_movie(user, i, key)].number_times--;
		
		for (i = 0; i < dataset->number_users; i++)
			users[i] = TRUE;
		
		qsort(most_greedy, dataset->number_items, sizeof(*most_greedy), CSP_generator_tree::number_times_cmp);
	}
	else
	{
		movie = most_greedy[which_one - 1].movie_id;
		
		qsort(most_greedy + which_one, dataset->number_items - which_one, sizeof(*most_greedy), CSP_generator_tree::movie_id_cmp);
		
		/*
			Of the people we're still considering, if they rated the same way keep considering their greedy results.
		*/
		for (other_user = 0; other_user < dataset->number_users; other_user++)
		{
			if (users[other_user])
			{
				user_ratings = dataset->ratings_for_user(other_user, &count);
				result = (uint64_t *)bsearch(&movie, user_ratings, count, sizeof(*user_ratings), CSP_generator_tree::movie_search);
				
				/*
					If they didn't rate the same way we did for this movie,
					then take their results out of consideration.
				*/
				if ((result && !key) || (!result && key))
				{
					users[other_user] = FALSE;
					for (i = 0; i < NUMCONSIDER; i++)
						most_greedy[CSP_generator_greedy_cheat::next_movie(other_user, i, key)].number_times--;
				}
			}
		}
		
		/*
			Resort by the number of times seen by people who have rated the same way we did
		*/
		qsort(most_greedy + which_one, dataset->number_items - which_one, sizeof(*most_greedy), CSP_generator_tree::number_times_cmp);
	}
	
	return most_greedy[which_one].movie_id;
}
