/*
	GENERATOR_NAIVE_BAYES.C
	-----------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "generator_naive_bayes.h"

/*
	CSP_GENERATOR_NAIVE_BAYES::CSP_GENERATOR_NAIVE_BAYES()
	------------------------------------------------------
*/
CSP_generator_naive_bayes::CSP_generator_naive_bayes(CSP_dataset *dataset, uint32_t *coraters) : CSP_generator_entropy(dataset), coraters(coraters)
{
}

/*
	CSP_GENERATOR_NAIVE_BAYES::PROBABILITY_CMP()
	--------------------------------------------
*/
int CSP_generator_naive_bayes::probability_cmp(const void *a, const void *b)
{
	UNUSED(a);
	UNUSED(b);
	return 0;
}

/*
	CSP_GENERATOR_NAIVE_BAYES::CALCULATE_PROBABILITY()
	--------------------------------------------------
*/
double CSP_generator_naive_bayes::calculate_probability(uint64_t movie, uint64_t non_ratable, uint64_t ratable)
{
	UNUSED(movie);
	UNUSED(non_ratable);
	UNUSED(ratable);
	return 1.0;
}

/*
	CSP_GENERATOR_NAIVE_BAYES::GENERATE()
	-------------------------------------
*/
uint64_t *CSP_generator_naive_bayes::generate(uint64_t user, uint64_t number_presented)
{
	uint64_t i;
	UNUSED(user);
	
	if (number_presented == 0)
		last_presented_and_seen = 0;
	else
	{
		printf("From %lu -> %lu was presented! ", last_presented_and_seen, number_presented - 1);
		printf("Saw %lu @ %lu\n", presentation_list[number_presented - 1], number_presented - 1);
		
		/*
			For each remaining item, need to update the probabilities we've seen them.
		*/
		for (i = number_presented; i < dataset->number_items; i++)
		{
		}
	}
	last_presented_and_seen = number_presented;
	
	return presentation_list;
}
