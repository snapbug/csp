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
	
	for (i = 0; i < dataset->number_items; i++)
		presentation_list[i] = i;
	
	shuffle(presentation_list, dataset->number_items);
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
uint64_t *CSP_generator_random::generate(uint64_t user, uint64_t number_presented)
{
	user = user;
	
	/*
		Shuffle the remaining list of mids.
		Sample for how we need to deal with this for dynamic generators.
	*/
	if (number_presented == 0)
		shuffle(presentation_list, dataset->number_items);
	
	return presentation_list;
}
