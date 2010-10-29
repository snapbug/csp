/*
	METRIC_RMSE.C
	-------------
*/
#include <math.h>
#include "metric_rmse.h"

/*
	CSP_METRIC_RMSE::CSP_METRIC_RMSE()
	----------------------------------
*/
CSP_metric_rmse::CSP_metric_rmse(CSP_dataset *dataset, CSP_predictor *predictor) : CSP_metric(dataset, predictor)
{
}

/*
	CSP_METRIC_RMSE::SCORE()
	------------------------
*/
double CSP_metric_rmse::score(uint64_t user)
{
	int64_t rating;
	uint64_t count;
	uint64_t *ratings = dataset->test_ratings_for_user(user, &count);
	double prediction, sum_squared_error = 0;
	
	#pragma omp parallel for private(prediction) reduction(+:sum_squared_error)
	for (rating = 0; rating < (int64_t)count; rating++)
	{
		prediction = predictor->predict(user, dataset->movie(ratings[rating]), dataset->day(ratings[rating]));
		prediction = clip(prediction, dataset->minimum, dataset->maximum);
		sum_squared_error += (prediction - dataset->rating(ratings[rating])) * (prediction - dataset->rating(ratings[rating]));
	}
	return sqrt(sum_squared_error / count);
}
