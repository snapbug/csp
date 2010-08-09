/*
	GENERATOR_RANDOM.C
	------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include "generator_random.h"

/*
	CSP_GENERATOR_RANDOM::CSP_GENERATOR_RANDOM()
	--------------------------------------------
*/
CSP_generator_random::CSP_generator_random(CSP_dataset *dataset) : CSP_generator(dataset)
{
	uint64_t i;
	
	mids = new uint64_t[dataset->number_items];
	
	for (i = 0; i < dataset->number_items; i++)
		mids[i] = i;
	
	shuffle(mids, dataset->number_items);
}

/*
	CSP_GENERATOR_RANDOM::SHUFFLE()
	-------------------------------
	Adapted from: http://www.stanford.edu/~blp/writings/clc/shuffle.html
*/
void CSP_generator_random::shuffle(uint64_t *start, uint64_t number)
{
	uint64_t i, j, temp;
	
	if (number <= 1)
		return;
	
	for (i = 0; i < number; i++)
	{
		j = (uint64_t)(i + rand() / (RAND_MAX / (double)(number - i) + 1));
		temp = start[j];
		start[j] = start[i];
		start[i] = temp;
	}
}

/*
	CSP_GENERATOR_RANDOM::NEXT_MOVIE()
	----------------------------------
*/
uint64_t CSP_generator_random::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	UNUSED(key);
	UNUSED(user);
	
	/*
		Shuffle the remaining list of mids.
		Sample for how we need to deal with this for dynamic generators.
	*/
	if (which_one == 0)
		shuffle(mids, dataset->number_items);
	
	return mids[which_one];
}
