/*
	PREDICTOR_ITEM_AVG.C
	--------------------
*/

#include <stdio.h>
#include "predictor_item_avg.h"

/*
	CSP_PREDICTOR_ITEM_AVG::CSP_PREDICTOR_ITEM_AVG()
	------------------------------------------------
*/
CSP_predictor_item_avg::CSP_predictor_item_avg(CSP_dataset *dataset) : CSP_predictor(dataset)
{
	double *number_ratings = new double[dataset->number_items];
	uint64_t *sum_of_ratings = new uint64_t[dataset->number_items];
	uint64_t num_ratings, i;
	uint64_t *data = dataset->get_ratings(&num_ratings);
	
	averages = new double[dataset->number_items];
	for (i = 0; i < dataset->number_items; i++)
	{
		number_ratings[i] = 0;
		sum_of_ratings[i] = 0;
	}
	
	for (i = 0; i < num_ratings; i++)
	{
		sum_of_ratings[dataset->movie(data[i])] += dataset->rating(data[i]);
		number_ratings[dataset->movie(data[i])]++;
	}

	for (i = 0; i < dataset->number_items; i++)
		averages[i] = sum_of_ratings[i] / number_ratings[i];
}

/*
	CSP_PREDICTOR_ITEM_AVG::PREDICT()
	---------------------------------
*/
double CSP_predictor_item_avg::predict(uint64_t user, uint64_t movie, uint64_t day)
{
	user = user;
	day = day;
	return averages[movie];
}
