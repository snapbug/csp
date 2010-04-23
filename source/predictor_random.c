/*
	PREDICTOR_RANDOM.C
	------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include "predictor_random.h"


double CSP_predictor_random::predict(uint64_t user, uint64_t movie, uint64_t day)
{
	user = user;
	movie = movie;
	day = day;
	// return a random value between min and max
	return dataset->minimum + rand() / (RAND_MAX / (double)(dataset->maximum - dataset->minimum) + 1);
}
