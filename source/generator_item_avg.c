/*
	GENERATOR_ITEM_AVG.C
	--------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include "generator_item_avg.h"

/*
	CSP_GENERATOR_ITEM_AVG::AVERAGE_CMP()
	-------------------------------------
	Used to sort movies by their average rating
*/
int CSP_generator_item_avg::average_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	return (x->average < y->average) - (x->average > y->average);
}

/*
	CSP_GENERATOR_ITEM_AVG::CSP_GENERATOR_ITEM_AVG()
	------------------------------------------------
*/
CSP_generator_item_avg::CSP_generator_item_avg(CSP_dataset *dataset) : CSP_generator(dataset)
{
	uint64_t i, count;
	uint64_t *ratings;
	
	most_liked = new movie[dataset->number_items];
	
	/*
		Regenerate the averages (might have changed since last generation).
	*/
	for (i = 0; i < dataset->number_items; i++)
	{
		most_liked[i].average = 0;
		most_liked[i].movie_id = i;
		most_liked[i].count = 0;
	}
	
	ratings = dataset->get_ratings(&count);
	
	for (i = 0; i < count; i++)
	{
		most_liked[dataset->movie(ratings[i])].count++;
		most_liked[dataset->movie(ratings[i])].average += (double)dataset->rating(ratings[i]);
	}
	
	for (i = 0; i < dataset->number_items; i++)
		most_liked[i].average /= most_liked[i].count;
	
	qsort(most_liked, dataset->number_items, sizeof(*most_liked), CSP_generator_item_avg::average_cmp);
}

/*
	CSP_GENERATOR_ITEM_AVG::NEXT_MOVIE()
	------------------------------------
*/
uint64_t CSP_generator_item_avg::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	UNUSED(key);
	UNUSED(user);
	
	return most_liked[which_one].movie_id;
}
