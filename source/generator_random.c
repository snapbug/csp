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
	CSP_GENERATOR_RANDOM::GENERATE()
	--------------------------------
*/
void CSP_generator_random::generate(uint64_t user, uint64_t *presentation_list, uint64_t number_presented)
{
	uint64_t i;
	UNUSED(user);
	
	/*
		Shuffle the remaining list of mids.
		Sample for how we need to deal with this for dynamic generators.
	*/
	if (number_presented == 0)
		shuffle(mids, dataset->number_items);
		
	for (i = number_presented; i < dataset->number_items; i++)
		presentation_list[i] = mids[i];
}
