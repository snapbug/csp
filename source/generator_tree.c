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
	
	if (x->included && !y->included) return -1;
	if (!x->included && y->included) return 1;
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
	uint64_t i, j, other_user, last_movie, other_movie, count, index, rating;
	uint64_t *movie_ratings;
	
	/*
		Reset all the counts
	*/
	for (i = 0; i < dataset->number_items; i++)
		most_greedy[i].number_times = 0;
	
	if (which_one == 0)
	{
		/*
			Reset all the counts and settings
		*/
		for (i = 0; i < dataset->number_items; i++)
		{
			most_greedy[i].movie_id = i;
			most_greedy[i].included = FALSE;
		}
		
		/*
			Count all but the user we're currently looking at
		*/
		for (i = 0; i < dataset->number_users; i++)
		{
			users[i] = TRUE;
			for (j = 0; j < NUMCONSIDER; j++)
				most_greedy[greedy_movies[(NUMDONE * i) + j]].number_times++;
		}
		users[user] = FALSE;
	}
	else
	{
		/*
			The last movie that was presented
		*/
		last_movie = most_greedy[which_one - 1].movie_id;
		
		/*
			Sort by movie id
		*/
		qsort(most_greedy, dataset->number_items, sizeof(*most_greedy), CSP_generator_tree::movie_id_cmp);
		
		/*
			Find which peoples greedy results to use.
		*/
		movie_ratings = dataset->ratings_for_movie(last_movie, &count);
		if (key) // find all people who could rate the previous movie and add them
		{
			for (rating = 0; rating < count; rating++)
			{
				other_user = dataset->user(movie_ratings[rating]);
				if (!users[other_user])
					continue;
				
				for (i = 0; i < NUMCONSIDER; i++)
				{
					other_movie = greedy_movies[(NUMDONE * other_user) + i];
					most_greedy[other_movie].number_times++;
				}
			}
		}
		else // find all people who couldn't rate the previous movie and add them
		{
			index = 0;
			for (other_user = 0; other_user < dataset->number_users; other_user++)
			{
				if (!users[other_user])
					continue;
				
				if (other_user < dataset->user(movie_ratings[index]))
				{
					for (i = 0; i < NUMCONSIDER; i++)
					{
						other_movie = greedy_movies[(NUMDONE * other_user) + i];
						most_greedy[other_movie].number_times++;
					}
				}
				else
				{
					index++;
				}
			}
		}
	}
	
	/*
		Sort by the number of times in other people's greedy list
	*/
	qsort(most_greedy, dataset->number_items, sizeof(*most_greedy), CSP_generator_tree::number_times_cmp);
	
	most_greedy[which_one].included = TRUE;
	return most_greedy[which_one].movie_id;
}
