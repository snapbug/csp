/*
	GENERATOR_ENTROPY.C
	-------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "generator_entropy.h"

//#define ENT

/*
	CSP_GENERATOR_ENTROPY::CSP_GENERATOR_ENTROPY()
	----------------------------------------------
	Implements Entropy0 as described by Rashid et. al. Learning Preferences of New Users in Recommender Systems: An Information Theoretic Approach. SIGKDD Eplr. Newsl., 10(2):90-100, 2008
*/
CSP_generator_entropy::CSP_generator_entropy(CSP_dataset *dataset) : CSP_generator(dataset)
{
	uint64_t i;
	
	most_entropic = new movie[dataset->number_items];
	weights = new double[dataset->maximum - dataset->minimum + 2]; // +2 to include 0 and maximum
	
	sum_weights = weights[0] = 0.5; // hard code non-votes to be 0.5 weight
	for (i = 1; i < dataset->maximum - dataset->minimum + 2; i++)
		sum_weights += weights[i] = 1;
	
	movie_counts = new uint64_t*[dataset->number_items];
	for (i = 0; i < dataset->number_items; i++)
		movie_counts[i] = new uint64_t[dataset->maximum - dataset->minimum + 2];

#if 0
	uint64_t j, rating, count;
	uint64_t *ratings;
	double prob_this_rating;
	/*
		Reset the counts.
	*/
	for (i = 0; i < dataset->number_items; i++)
	{
		for (j = dataset->minimum; j < dataset->maximum + 2; j++)
			movie_counts[i][j - dataset->minimum] = 0;
		most_entropic[i].entropy = 0;
		most_entropic[i].ratings = 0;
		most_entropic[i].movie_id = i;
	}
	
	/*
		Accumulate the counts.
	*/
	ratings = dataset->get_ratings(&count);
	for (i = 0; i < count; i++)
	{
		most_entropic[dataset->movie(ratings[i])].ratings++;
		movie_counts[dataset->movie(ratings[i])][dataset->rating(ratings[i])]++;
	}
	
	/*
		Calculate the Entropy0 of every item
	*/
	for (i = 0; i < dataset->number_items; i++)
	{
#ifdef ENT
		movie_counts[i][0] = 0;
#else
		movie_counts[i][0] = dataset->number_users - most_entropic[i].ratings;
#endif
	
		/*
			Calculate the entropy of ratings for this movie.
		*/
#ifdef ENT
		for (j = dataset->minimum + 1; j < dataset->maximum + 2; j++)
#else
		for (j = dataset->minimum; j < dataset->maximum + 2; j++)
#endif
		{
			rating = j - dataset->minimum;
			prob_this_rating = 1.0 * movie_counts[i][rating] / (most_entropic[i].ratings + movie_counts[i][0]);
			
#ifdef ENT
			most_entropic[i].entropy += prob_this_rating * (prob_this_rating ? log(prob_this_rating) : 0);
#else
			most_entropic[i].entropy += weights[rating] * prob_this_rating * (prob_this_rating ? log(prob_this_rating) : 0);
#endif
		}
		
#ifdef ENT
		most_entropic[i].entropy *= -1;
#else
		most_entropic[i].entropy = -most_entropic[i].entropy / sum_weights;
#endif
		/*
			Uncomment for log(pop) * entropy as described in same paper.
		*/
		//most_entropic[i].entropy = most_entropic[i].entropy * log(most_entropic[i].ratings);
	}
	
	qsort(most_entropic, dataset->number_items, sizeof(*most_entropic), CSP_generator_entropy::entropy_cmp);
	
	for (i = 0; i < dataset->number_items; i++)
		printf("%lu %f\n", most_entropic[i].movie_id, most_entropic[i].entropy);
	exit(EXIT_SUCCESS);
#endif
}

/*
	CSP_GENERATOR_ENTROPY::ENTROPY_CMP()
	------------------------------------
	Used to sort movies by their entropy
*/
int CSP_generator_entropy::entropy_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	return (x->entropy < y->entropy) - (x->entropy > y->entropy);
}

/*
	CSP_GENERATOR_ENTROPY::NEXT_MOVIE()
	-----------------------------------
*/
uint64_t CSP_generator_entropy::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	uint64_t i, j, count, rating;
	uint64_t *ratings;
	double prob_this_rating;
	
	UNUSED(key);
	UNUSED(user);
	
	if (which_one == 0)
	{
		/*
			Reset the counts.
		*/
		for (i = 0; i < dataset->number_items; i++)
		{
			for (j = dataset->minimum; j < dataset->maximum + 2; j++)
				movie_counts[i][j - dataset->minimum] = 0;
			most_entropic[i].entropy = 0;
			most_entropic[i].ratings = 0;
			most_entropic[i].movie_id = i;
		}
		
		/*
			Accumulate the counts.
		*/
		ratings = dataset->get_ratings(&count);
		for (i = 0; i < count; i++)
		{
			if (dataset->included(ratings[i]))
			{
				most_entropic[dataset->movie(ratings[i])].ratings++;
				movie_counts[dataset->movie(ratings[i])][dataset->rating(ratings[i])]++;
			}
		}
		
		/*
			Calculate the Entropy0 of every item
		*/
		for (i = 0; i < dataset->number_items; i++)
		{
#ifdef ENT
			movie_counts[i][0] = 0;
#else
			movie_counts[i][0] = dataset->number_users - most_entropic[i].ratings;
#endif
		
			/*
				Calculate the entropy of ratings for this movie.
			*/
#ifdef ENT
			for (j = dataset->minimum + 1; j < dataset->maximum + 2; j++)
#else
			for (j = dataset->minimum; j < dataset->maximum + 2; j++)
#endif
			{
				rating = j - dataset->minimum;
				prob_this_rating = 1.0 * movie_counts[i][rating] / (most_entropic[i].ratings + movie_counts[i][0]);
				
#ifdef ENT
				most_entropic[i].entropy += prob_this_rating * (prob_this_rating ? log(prob_this_rating) : 0);
#else
				most_entropic[i].entropy += weights[rating] * prob_this_rating * (prob_this_rating ? log(prob_this_rating) : 0);
#endif
			}
			
#ifdef ENT
			most_entropic[i].entropy *= -1;
#else
			most_entropic[i].entropy = -most_entropic[i].entropy / sum_weights;
#endif
			/*
				Uncomment for log(pop) * entropy as described in same paper.
			*/
			//most_entropic[i].entropy = most_entropic[i].entropy * log(most_entropic[i].ratings);
		}
		
		qsort(most_entropic, dataset->number_items, sizeof(*most_entropic), CSP_generator_entropy::entropy_cmp);
	}
	
	return most_entropic[which_one].movie_id;
}
