/*
	GENERATOR_DISTANCE.C
	--------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "generator_distance.h"

/*
	CSP_GENERATOR_DISTANCE::DISTANCE_CMP()
	--------------------------------------
	Used to sort movies by their distance
*/
int CSP_generator_distance::distance_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	if (x->distance < y->distance) return 1;
	if (x->distance > y->distance) return -1;
	return 0;
}

/*
	CSP_GENERATOR_DISTANCE::CSP_GENERATOR_DISTANCE()
	------------------------------------------------
*/
CSP_generator_distance::CSP_generator_distance(CSP_dataset *dataset) : CSP_generator(dataset)
{
	double *averages;
	uint64_t *ratings, count, rating, i;
	
	most_distant = new movie[dataset->number_items];
	averages = new double[dataset->number_items];
	
	for (i = 0; i < dataset->number_items; i++)
	{
		ratings = dataset->ratings_for_movie(i, &count);
		most_distant[i].movie_id = i;
		most_distant[i].distance = 0;
		averages[i] = 0;
		
		for (rating = 0; rating < count; rating++)
			averages[i] += dataset->rating(ratings[rating]);
		averages[i] /= count;
		
		for (rating = 0; rating < count; rating++)
			most_distant[i].distance += pow(dataset->rating(ratings[rating]) - averages[i], 2);
		most_distant[i].distance /= count;
		most_distant[i].distance *= count / (count + 4000.);
	}
	
	qsort(most_distant, dataset->number_items, sizeof(*most_distant), CSP_generator_distance::distance_cmp);
	
	for (i = 0; i < dataset->number_items; i++)
		presentation_list[i] = most_distant[i].movie_id;
}

/*
	CSP_GENERATOR_DISTANCE::GENERATE()
	----------------------------------
*/
uint64_t *CSP_generator_distance::generate(uint64_t user, uint64_t number_presented)
{
	UNUSED(user);
	UNUSED(number_presented);
	
	return presentation_list;
}
