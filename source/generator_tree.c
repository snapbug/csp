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
	uint64_t x = *(uint64_t *)a;
	uint64_t y = *(uint64_t *)b;
	
	return (x > y) - (x < y);
}

/*
	CSP_GENERATOR_TREE::MOVIE_USER_SEARCH()
	---------------------------------------
	Used to find a movie id in a users ratings
*/
int CSP_generator_tree::movie_user_search(const void *a, const void *b)
{
	uint64_t key = *(uint64_t *)a;
	uint64_t item = (*(uint64_t *)b) >> 15 & 32767;
	return (key > item) - (key < item);
}

/*
	CSP_GENERATOR_TREE::MOVIE_GREEDY_SEARCH()
	-----------------------------------------
	Used to find a movie id in a list of greedy thing
*/
int CSP_generator_tree::movie_greedy_search(const void *a, const void *b)
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
	uint64_t i, other_user, last_movie, other_movie, count;
	uint64_t *user_ratings, *result;
	movie *greedy;
	
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
			Count all but the user we're currently looking at
		*/
		for (i = 0; i < dataset->number_users; i++)
		{
			users[i] = TRUE;
			for (j = 0; j < NUMCONSIDER && i != user; j++)
				most_greedy[greedy_movies[(NUMDONE * i) + j]].number_times++;
		}
		
		/*
			Consider all users, apart from the current
		*/
		users[user] = FALSE;
		
		/*
			Sort by the number of times it appeared in other people's greedy lists
		*/
		qsort(most_greedy, dataset->number_items, sizeof(*most_greedy), CSP_generator_tree::number_times_cmp);
	}
	else
	{
		/*
			Sort by movie id so we can bsearch later on
		*/
		qsort(most_greedy + which_one, dataset->number_items - which_one, sizeof(*most_greedy), CSP_generator_tree::movie_id_cmp);
		
		/*
			The last movie that was presented
		*/
		last_movie = most_greedy[which_one - 1].movie_id;
		
		/*
			Of the people we're still considering, if they rated the same way keep considering their greedy results
		*/
		for (other_user = 0; other_user < dataset->number_users; other_user++)
		{
			if (users[other_user])
			{
				user_ratings = dataset->ratings_for_user(other_user, &count);
				
				/*
					Search in their ratings for this movie
				*/
				result = (uint64_t *)bsearch(&last_movie, user_ratings, count, sizeof(*user_ratings), CSP_generator_tree::movie_user_search);
				
				/*
					If they weren't able to rate the same way, then take their results out of consideration
				*/
				if ((result && !key) || (!result && key))
				{
					/*
						Don't look at them again
					*/
					users[other_user] = FALSE;
					
					/*
						Remove the ones that greedy would choose for them
					*/
					for (i = 0; i < NUMCONSIDER; i++)
					{
						/*
							Get the movie that the other user got for greedy in position
						*/
						other_movie = greedy_movies[(NUMDONE * other_user) + i];
						
						/*
							Find it in the most greedy list and remove if in the second half
						*/
						greedy = (movie *)bsearch(&other_movie, most_greedy + which_one, dataset->number_items - which_one, sizeof(*most_greedy), CSP_generator_tree::movie_greedy_search);
						if (greedy) greedy->number_times--;
					}
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
