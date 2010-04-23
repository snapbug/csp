/*
	PREDICTOR_GLOBAL_AVG.C
	----------------------
*/

#include <stdio.h>
#include "predictor_global_avg.h"

/*
	CSP_PREDICTOR_GLOBAL_AVG::CSP_PREDICTOR_GLOBAL_AVG()
	----------------------------------------------------
*/
CSP_predictor_global_avg::CSP_predictor_global_avg(CSP_dataset *dataset) : CSP_predictor(dataset)
{
	uint64_t i;
	uint64_t *data = dataset->get_ratings(&number_ratings);

	for (i = 0; i < number_ratings; i++)
		sum_of_ratings += (double)dataset->rating(data[i]);
	printf("%f\n", sum_of_ratings / number_ratings);
}

/*
	CSP_PREDICTOR_GLOBAL_AVG::ADDED_RATING()
	----------------------------------------
*/
void CSP_predictor_global_avg::added_rating(uint64_t *key)
{
	sum_of_ratings += (double)dataset->rating(key);
	number_ratings++;
}

/*
	CSP_PREDICTOR_GLOBAL_AVG::REMOVED_RATING()
	------------------------------------------
*/
void CSP_predictor_global_avg::removed_rating(uint64_t *key)
{
	sum_of_ratings -= (double)dataset->rating(key);
	number_ratings--;
}

/*
	CSP_PREDICTOR_GLOBAL_AVG::PREDICT()
	-----------------------------------
*/
double CSP_predictor_global_avg::predict(uint64_t user, uint64_t movie, uint64_t day)
{
	user = user;
	movie = movie;
	day = day;
	return sum_of_ratings / number_ratings;
}
