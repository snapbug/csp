/*
	GENERATOR_POPULARITY.C
	----------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include "generator_popularity.h"

/*
	CSP_GENERATOR_POPULARITY::NUMBER_RATINGS_CMP()
	----------------------------------------------
	Used to sort movies by the number of ratings it got
*/
int CSP_generator_popularity::number_ratings_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	return (x->number_ratings < y->number_ratings) - (x->number_ratings > y->number_ratings);
}

/*
	CSP_GENERATOR_POPULARITY::CSP_GENERATOR_POPULARITY()
	----------------------------------------------------
*/
CSP_generator_popularity::CSP_generator_popularity(CSP_dataset *dataset) : CSP_generator(dataset)
{
	uint64_t i, count;
	uint64_t *ratings;
	
	most_popular = new movie[dataset->number_items];
	for (i = 0; i < dataset->number_items; i++)
	{
		most_popular[i].number_ratings = 0;
		most_popular[i].movie_id = i;
	}
	
	ratings = dataset->get_ratings(&count);
	
	for (i = 0; i < count; i++)
		most_popular[dataset->movie(ratings[i])].number_ratings++;
		
	qsort(most_popular, dataset->number_items, sizeof(*most_popular), CSP_generator_popularity::number_ratings_cmp);
}

/*
	CSP_GENERATOR_POPULARITY::NEXT_MOVIE()
	--------------------------------------
*/
uint64_t CSP_generator_popularity::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	UNUSED(key);
	UNUSED(user);
	
	return most_popular[which_one].movie_id;
}
