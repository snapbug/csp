/*
	GENERATOR_TREE.C
	----------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "generator_tree.h"

//#define SIMULATE

/*
	CSP_GENERATOR_TREE::CSP_GENERATOR_TREE()
	----------------------------------------
*/
CSP_generator_tree::CSP_generator_tree(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric) : CSP_generator_greedy_cheat(dataset, predictor, metric)
{
	history_len = 1;
	most_greedy = new movie[dataset->number_items];
	users = new uint64_t[dataset->number_users];
	history = new uint64_t[dataset->number_items];
	
#ifdef SIMULATE
	uint64_t i, j, k, nm, nd, sum, dud;
	double level = 7;
	
	fprintf(stderr, "Doing history %lu\n", history_len);
	
	for (i = (uint64_t)pow(2.0, (2 * level) - 1); i < (uint64_t)pow(2.0, (2 * level)); i++)
	{
		j = i << 2; // make a dummy to pass for first instance
		nd = 0;
		while (j)
		{
			if (nd && (j & 1))
				nm = next_movie(0, nd, &(dud = (j & 2) ? 4 : 3)); // if second bit is set, simulate a 'low' (1,2,3) rating or 'high' rating (4,5)
			else
				nm = next_movie(0, nd, NULL); // NULL because it's either the first, or couldn't see the last one
			
			sum = 0;
			for (k = 0; k < dataset->number_users; k++)
				sum += users[k] ? 1 : 0;
			
			if (nd && (j & 1))
				printf("%d %lu %lu\n", j & 2 ? 2 : 1, nm, sum); // prints either high seeing, or low seeing
			else
				printf("%lu %lu %lu\n", j & 1, nm, sum); // prints non seeing
			
			j >>= 2;
			nd++;
		}
		printf("\n");
	}
	exit(EXIT_SUCCESS);
#endif
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
	CSP_GENERATOR_TREE::NEXT_MOVIE()
	--------------------------------
*/
uint64_t CSP_generator_tree::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	uint64_t i, j, other_user, count, rating, movie_index;
	uint64_t *movie_ratings;
	uint64_t replaced_filter = FALSE;
	
	/*
		Reset all the counts and settings
	*/
	if (which_one == 0)
	{
		for (i = 0; i < dataset->number_items; i++)
		{
			most_greedy[i].included = FALSE;
			most_greedy[i].movie_id = i;
			history[i] = dataset->number_items << 15;
		}
	}
	
	/*
		Reset the id/number of times for each movie.
	*/
	for (i = 0; i < dataset->number_items; i++)
		most_greedy[i].number_times = 0;
	
	/*
		Only update the filter if we're replacing an old filter
	*/
	if (which_one == 0 || dataset->movie(history[(which_one - 1) % history_len]) < dataset->number_items)
	{
		/*
			Count all but the user we're currently looking at
		*/
		for (i = 0; i < dataset->number_users; i++)
			users[i] = TRUE;
#ifndef SIMULATE
		users[user] = FALSE;
#endif
		replaced_filter = TRUE;
	}
	
	/*
		Update history of the last n items
	*/
	if (which_one > 0)
		history[(which_one - 1) % history_len] = key ? *key : (most_greedy[which_one - 1].movie_id << 15);
		
	/*
		For each movie we've presented in the history, filter the users
		If we've not replaced, then only consider the last filter, not all
	*/
	for (i = (replaced_filter ? 0 : (which_one - 1) % history_len); i < history_len; i++)
	{
		if (dataset->movie(history[i]) < dataset->number_items)
		{
			rating = dataset->rating(history[i]);
			movie_ratings = dataset->ratings_for_movie(dataset->movie(history[i]), &count);
			movie_index = 0;
			
			for (other_user = 0; other_user < dataset->number_users; other_user++)
			{
				/*
					other_user is now someone that could see the movie
				*/
				if (movie_index < count && other_user == dataset->user(movie_ratings[movie_index])) // we saw, they saw
				{
					/*
						Only consider if we already are, otherwise could change a FALSE to TRUE
					*/
					if (users[other_user])
						users[other_user] = rating && ((rating > 3) == (dataset->rating(movie_ratings[movie_index]) > 3)); // rating 'parity' the same
					
					/*
						Move onto the next person that could see movie
					*/
					movie_index++;
				}
				else if (rating) // we saw, they didn't
				{
					users[other_user] = FALSE;
				}
			}
		}
	}
	
	/*
		Sort by movie id so addition works
	*/
	qsort(most_greedy, dataset->number_items, sizeof(*most_greedy), CSP_generator_tree::movie_id_cmp);
	
	/*
		Now having filtered the users, count the greedy choices.
	*/
	for (i = 0; i < dataset->number_users; i++)
		for (j = 0; users[i] && j < NUMCONSIDER; j++)
			most_greedy[greedy_movies[(i * NUMDONE) + j]].number_times++;
	
	/*
		Sort by the number of times in other people's greedy list
	*/
	qsort(most_greedy, dataset->number_items, sizeof(*most_greedy), CSP_generator_tree::number_times_cmp);
	
	/*
		Set the one we're going to present to have been already included so we don't do it again
	*/
	most_greedy[which_one].included = TRUE;
	return most_greedy[which_one].movie_id;
}
