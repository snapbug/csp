/*
	METRIC_MAE.C
	------------
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "metric_mae.h"

/*
	CSP_METRIC_MAE::CSP_METRIC_MAE()
	--------------------------------
*/
CSP_metric_mae::CSP_metric_mae(CSP_dataset *dataset, CSP_predictor *predictor) : CSP_metric(dataset, predictor)
{
//	predictions_made = 0;
//	sum_absolute_error = 0;
}

/*
	CSP_METRIC_MAE::UPDATE()
	------------------------
*/
//void CSP_metric_mae::update(double predicted, double actual)
//{
//	sum_absolute_error += fabs(predicted - actual);
//	predictions_made++;
//}

/*
	CSP_METRIC_MAE::SCORE()
	-----------------------
*/
double CSP_metric_mae::score(uint64_t user)
{
	int64_t rating;
	uint64_t count;
	uint64_t *ratings = dataset->test_ratings_for_user(user, &count);
	double prediction, sum_absolute_error = 0;
	
	#pragma omp parallel for private(prediction) reduction(+:sum_absolute_error)
	for (rating = 0; rating < (int64_t)count; rating++)
	{
		prediction = predictor->predict(user, dataset->movie(ratings[rating]), dataset->day(ratings[rating]));
		prediction = clip(prediction, dataset->minimum, dataset->maximum);
		sum_absolute_error += fabs(prediction - dataset->rating(ratings[rating]));
	}
	return sum_absolute_error / count;
}

/*
	CSP_METRIC_MAE::RESET()
	-----------------------
*/
//void CSP_metric_mae::reset(void)
//{
//	sum_absolute_error = 0;
//	predictions_made = 0;
//}
