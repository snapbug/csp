/*
	GENERATOR_PREDICTOR.C
	---------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include "generator_predictor.h"

/*
	CSP_GENERATOR_PREDICTOR::CSP_GENERATOR_PREDICTOR()
	--------------------------------------------------
*/
CSP_generator_predictor::CSP_generator_predictor(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric) : CSP_generator(dataset), metric(metric), predictor(predictor) 
{
	uint64_t i;
	error_reduction = new movie[dataset->number_items];
	for (i = 0; i < dataset->number_items; i++)
		error_reduction[i].movie_id = i;
}

/*
	CSP_GENERATOR_PREDICTOR::ERROR_CMP()
	------------------------------------
*/
int CSP_generator_predictor::error_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	return (x->prediction < y->prediction) - (x->prediction > y->prediction);
}

/*
	CSP_GENERATOR_PREDICTOR::NEXT_MOVIE()
	-------------------------------------
*/
uint64_t CSP_generator_predictor::next_movie(uint64_t user, uint64_t which_one, uint64_t *key)
{
	int64_t i;
	
	/*
		For each rating, if it hasn't been added, see what error we'd get
	*/
	if (key)
	{
		#pragma omp parallel for
		for (i = which_one; (uint64_t)i < dataset->number_items; i++)
			error_reduction[i].prediction = predictor->predict(user, error_reduction[i].movie_id, 2242);
	}
	
	qsort(error_reduction + which_one, dataset->number_items - which_one, sizeof(*error_reduction), CSP_generator_predictor::error_cmp);
	
	return error_reduction[which_one].movie_id;
}
