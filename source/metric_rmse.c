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
CSP_metric_rmse::CSP_metric_rmse() : CSP_metric()
{
	predictions_made = 0;
	sum_sqaured_error = 0;
}

/*
	CSP_METRIC_RMSE::UPDATE()
	-------------------------
*/
void CSP_metric_rmse::update(double predicted, double actual)
{
	sum_sqaured_error += pow(predicted - actual, 2);
	predictions_made++;
}

/*
	CSP_METRIC_RMSE::SCORE()
	------------------------
*/
double CSP_metric_rmse::score(void)
{
	return sqrt(sum_sqaured_error / predictions_made);
}

/*
	CSP_METRIC_RMSE::RESET()
	------------------------
*/
void CSP_metric_rmse::reset(void)
{
	sum_sqaured_error = 0;
	predictions_made = 0;
}
