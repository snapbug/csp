/*
	GENERATOR_TREE.C
	----------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "generator_tree.h"

#ifndef log2
	#define log2(x) (log(1.0 * (x)) / log(2.0))
#endif

#define TREE_ONCE

/*
	CSP_GENERATOR_TREE::CSP_GENERATOR_TREE()
	----------------------------------------
*/
CSP_generator_tree::CSP_generator_tree(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric) : CSP_generator_greedy_cheat(dataset, predictor, metric)
{
	uint64_t i, j, k, nm, nd, sum;
	double level = 5;
	
	most_greedy = new movie[dataset->number_items];
	users = new uint64_t[dataset->number_users];
	
#ifdef TREE_ONCE
	fprintf(stderr, "DOING TREE_ONCE\n");
#else
	fprintf(stderr, "DOING SPLIT_ALWYAS\n");
#endif
	
	for (i = (uint64_t)pow(2.0, level - 1); i < (uint64_t)pow(2.0, level); i++)
	{
		j = i << 1;
		nd = 0;
		while (j)
		{
			nm = next_movie(0, nd, j & 1 ? &j : NULL);
			
			sum = 0;
			for (k = 0; k < dataset->number_users; k++)
				sum += users[k] ? 1 : 0;
			
			printf("%lu %lu %lu\n", j & 1, nm, sum);
			j >>= 1;
			nd++;
		}
		printf("\n");
	}
	exit(EXIT_SUCCESS);
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
	
#ifdef TREE_ONCE
	for (i = 0; i < dataset->number_items; i++)
		most_greedy[i].number_times = 0;
#endif
	
	if (which_one == 0)
	{
		/*
			Reset all the counts and settings
		*/
		for (i = 0; i < dataset->number_items; i++)
		{
			most_greedy[i].movie_id = i;
			most_greedy[i].included = FALSE;
#ifndef TREE_ONCE
			most_greedy[i].number_times = 0;
#endif
		}
		
		/*
			Count all but the user we're currently looking at
		*/
		for (i = 0; i < dataset->number_users; i++)
		{
			users[i] = TRUE;
#ifndef TREE_ONCE
			for (j = 0; j < NUMCONSIDER; j++)
				most_greedy[greedy_movies[(i * NUMDONE) + j]].number_times++;
#endif
		}
		//users[user] = FALSE;
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
#ifdef TREE_ONCE
		if (key) // find people that could rate the same way, add them
#else
		if (!key) // find people who couldn't rate the same way, remove them
#endif
		{
			for (rating = 0; rating < count; rating++)
			{
				other_user = dataset->user(movie_ratings[rating]);
				if (users[other_user])
				{
#ifndef TREE_ONCE
					users[other_user] = FALSE; // don't look at them again
#endif
					
					for (i = 0; i < NUMCONSIDER; i++)
					{
						other_movie = greedy_movies[(NUMDONE * other_user) + i];
#ifdef TREE_ONCE
						most_greedy[other_movie].number_times++;
#else
						most_greedy[other_movie].number_times--;
#endif
					}
				}
			}
		}
		else
		{
			other_user = index = 0;
			for (rating = 0; rating < count; rating++)
			{
				index = dataset->user(movie_ratings[rating]);
				while (other_user < index)
				{
					if (users[other_user])
					{
#ifndef TREE_ONCE
						users[other_user] = FALSE; // don't look at them again
#endif
						
						for (i = 0; i < NUMCONSIDER; i++)
						{
							other_movie = greedy_movies[(NUMDONE * other_user) + i];
#ifdef TREE_ONCE
							most_greedy[other_movie].number_times++;
#else
							most_greedy[other_movie].number_times--;
#endif
						}
					}
					other_user++;
				}
				other_user++; // skip over other_user == index
			}
			while (other_user < dataset->number_users)
			{
				if (users[other_user])
				{
#ifndef TREE_ONCE
					users[other_user] = FALSE; // don't look at them again
#endif
					
					for (i = 0; i < NUMCONSIDER; i++)
					{
						other_movie = greedy_movies[(NUMDONE * other_user) + i];
#ifdef TREE_ONCE
						most_greedy[other_movie].number_times++;
#else
						most_greedy[other_movie].number_times--;
#endif
					}
				}
				other_user++;
			}
		}
	}
	
	/*
		Sort by the number of times in other people's greedy list
	*/
	qsort(most_greedy, dataset->number_items, sizeof(*most_greedy), CSP_generator_tree::number_times_cmp);
	
	/*
		All the non-included are first.
	*/
	most_greedy[which_one].included = TRUE;
	return most_greedy[which_one].movie_id;
}
