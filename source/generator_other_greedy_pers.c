/*
	GENERATOR_OTHER_GREEDY_PERS.C
	------------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include "generator_other_greedy_pers.h"

/*
	CSP_GENERATOR_OTHER_GREEDY_PERS::CSP_GENERATOR_OTHER_GREEDY_PERS()
	------------------------------------------------------------------
*/
CSP_generator_other_greedy_pers::CSP_generator_other_greedy_pers(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric) : CSP_generator_other_greedy(dataset, predictor, metric), metric(metric), predictor(predictor)
{
}

/*
	CSP_GENERATOR_OTHER_GREEDY_PERS::NEXT_MOVIE()
	---------------------------------------------
*/
uint64_t CSP_generator_other_greedy_pers::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	return CSP_generator_other_greedy::next_movie(user, which_one, key);
}
