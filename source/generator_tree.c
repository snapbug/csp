/*
	GENERATOR_TREE.C
	----------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "generator_tree.h"

//#define SIMULATE
//#define P_RESTART (dataset->number_users / 20)
//#define F_RESTART 10

/*
	CSP_GENERATOR_TREE::CSP_GENERATOR_TREE()
	----------------------------------------
*/
CSP_generator_tree::CSP_generator_tree(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric) : CSP_generator_greedy_cheat(dataset, predictor, metric)
{
#ifdef HISTORY
	history_len = HISTORY;
#else
	history_len = 10;
#endif
	most_greedy = new movie[dataset->number_items];
	users = new uint64_t[dataset->number_users];
	history = new uint64_t[dataset->number_items];
	
#if 0//def SIMULATE
	/*
		Tests all combinations of ratings for the tree.
		There are undoubtedly repeated paths, ie.
		1, 2 vs. 1, 3 for HIGH_CUT, but this way we can
		change how we split without worrying about changing
		the simulation.
	*/
	uint64_t i, j, other, present, num_done, num_others, rating;
	uint64_t valid;
	double level = 6;
	
	for (i = (uint64_t)pow(2.0, (3 * level) - 1); i < (uint64_t)pow(2.0, (3 * level)); i++)
	{
		valid = TRUE;
		
		j = i;
		while (j)
		{
			valid = valid && ((j & 7) < 6);
			j >>= 3;
		}
		
		if (!valid)
			continue;
		
		// make a dummy to pass for first instance
		j = i << 3;
		
		num_done = 0;
		while (j)
		{
			present = next_movie(0, num_done, num_done ? &(rating = j & 7) : NULL);
			
			num_others = 0;
			for (other = 0; other < dataset->number_users; other++)
				num_others += users[other] ? 1 : 0;
			
			/*
				prints the rating given to last movie, the
				next movie to present, and how many other
				users follow this path
			*/
			if (num_done)
				printf("%lu %lu %lu\n", rating, present, num_others);
			else
				printf("- %lu %lu\n", present, num_others);
			
			j >>= 3;
			num_done++;
		}
		printf("\n");
	}
	exit(EXIT_SUCCESS);
#endif
}

/*
	CSP_GENERATOR_TREE::PARITY()
	----------------------------
*/
int CSP_generator_tree::parity(uint64_t rating)
{
#define HIGH_CUT 3
#define MIDPOINT 3

#ifdef PARITY
	return PARITY;
#else
	return rating; // 0 1 2 3 4 5
#endif
	
	return rating; // 0 1 2 3 4 5
	return (rating > MIDPOINT) - (rating < MIDPOINT); // 0 12 3 45
	return rating > HIGH_CUT; // 0 123 45
	return rating == 0; // 0 12345
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
	uint64_t replaced_filter = which_one > history_len;
	uint64_t other_parity, my_parity;

	/*
		Filters only get replaced if using history length
	*/
#ifdef F_RESTART
	replaced_filter = FALSE;
#endif
#ifdef P_RESTART
	replaced_filter = FALSE;
#endif
	
	/*
		Reset all the settings
	*/
	if (which_one == 0)
	{
		for (i = 0; i < dataset->number_items; i++)
		{
			most_greedy[i].included = FALSE;
			most_greedy[i].movie_id = i;

#ifdef ML
			history[i] = dataset->number_items << 4;
#else
			history[i] = dataset->number_items << 15;
#endif
		}
		
		for (i = 0; i < dataset->number_users; i++)
			users[i] = TRUE;
#ifndef SIMULATE
		users[user] = FALSE;
#endif
	}
	
	/*
		Update history for the last item
	*/
	if (which_one > 0)
#ifdef ML
		history[which_one - 1] = (most_greedy[which_one - 1].movie_id << 4) | (key ? dataset->rating(key) : 0);
#else
		history[which_one - 1] = (most_greedy[which_one - 1].movie_id << 15) | (key ? dataset->rating(key) : 0);
#endif
	
	/*
		If we've replaced an older filter, add back the people that were affected by the old filter
	*/
	if (replaced_filter)
	{
		rating = dataset->rating(history[which_one - history_len - 1]);
		
		my_parity = parity(rating);
		
		movie_ratings = dataset->ratings_for_movie(dataset->movie(history[which_one - history_len - 1]), &count);
		movie_index = 0;
		
		for (other_user = 0; other_user < dataset->number_users; other_user++)
		{
			if (movie_index < count && other_user == dataset->user(movie_ratings[movie_index])) // other_user saw it
			{
				other_parity = parity(dataset->rating(movie_ratings[movie_index]));
				
				// they saw it and we didn't, or, we saw it and gave a different high/low
				if (!rating || (my_parity != other_parity))
					users[other_user] = TRUE;
				movie_index++;
			}
			else if (rating) // we saw it, they didn't, so now we want to reconsider them
			{
				users[other_user] = TRUE;
			}
		}
	}
	
	/*
		For each movie we've presented in the history, filter the users
		If we've not replaced a filter, we only need to update to the last one
	*/
	for (i = which_one - (replaced_filter ? history_len : 1); i < which_one; i++)
	{
		if (dataset->movie(history[i]) < dataset->number_items)
		{
			rating = dataset->rating(history[i]);
			
			my_parity = parity(rating);
			
			movie_ratings = dataset->ratings_for_movie(dataset->movie(history[i]), &count);
			movie_index = 0;
			
			for (other_user = 0; other_user < dataset->number_users; other_user++)
			{
				/*
					other_user is now someone that could see the movie
				*/
				if (movie_index < count && other_user == dataset->user(movie_ratings[movie_index])) // they saw
				{
					/*
						Only check if we already are considering, otherwise we could change FALSE to TRUE
					*/
					other_parity = parity(dataset->rating(movie_ratings[movie_index]));
					
					if (users[other_user]) // check rating 'parity' the same
						users[other_user] = rating && (my_parity == other_parity);
					
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
	
#ifdef P_RESTART
	uint64_t sum = 0;
	for (i = 0; i < dataset->number_users; i++)
		sum += users[i] ? 1 : 0;
	
	if (sum < P_RESTART)
	{
		for (i = 0; i < dataset->number_users; i++)
			users[i] = TRUE;
#ifndef SIMULATE
		users[user] = FALSE;
#endif
	}
#endif
	
#ifdef F_RESTART
	if (which_one % F_RESTART == 0)
	{
		for (i = 0; i < dataset->number_users; i++)
			users[i] = TRUE;
#ifndef SIMULATE
		users[user] = FALSE;
#endif
	}
#endif
	
	/*
		Reset the number of times for each movie.
	*/
	for (i = 0; i < dataset->number_items; i++)
		most_greedy[i].number_times = 0;
	
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
