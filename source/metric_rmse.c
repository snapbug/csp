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
//	predictions_made = 0;
//	sum_sqaured_error = 0;
}

/*
	CSP_METRIC_RMSE::UPDATE()
	-------------------------
*/
//void CSP_metric_rmse::update(double predicted, double actual)
//{
//	sum_sqaured_error += pow(predicted - actual, 2);
//	predictions_made++;
//}

/*
	CSP_METRIC_RMSE::SCORE()
	------------------------
*/
double CSP_metric_rmse::score(uint64_t user)
{
	uint64_t count, rating;
	uint64_t *ratings = dataset->test_ratings_for_user(user, &count);
	double prediction, sum_squared_error = 0;
	
	for (rating = 0; rating < count; rating++)
	{
		prediction = predictor->predict(user, dataset->movie(ratings[rating]), dataset->day(ratings[rating]));
		prediction = clip(prediction, dataset->minimum, dataset->maximum);
		sum_squared_error += pow(prediction - dataset->rating(ratings[rating]), 2);
	}
	return sum_squared_error / count;
}

/*
	CSP_METRIC_RMSE::RESET()
	------------------------
*/
//void CSP_metric_rmse::reset(void)
//{
//	sum_sqaured_error = 0;
//	predictions_made = 0;
//}
