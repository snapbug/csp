/*
	METRIC_MAE.C
	------------
*/

#include <math.h>
#include "metric_mae.h"

/*
	CSP_METRIC_MAE::CSP_METRIC_MAE()
	--------------------------------
*/
CSP_metric_mae::CSP_metric_mae() : CSP_metric()
{
	predictions_made = 0;
	sum_absolute_error = 0;
}

/*
	CSP_METRIC_MAE::UPDATE()
	------------------------
*/
void CSP_metric_mae::update(double predicted, double actual)
{
	sum_absolute_error += fabs(predicted - actual);
	predictions_made++;
}

/*
	CSP_METRIC_MAE::SCORE()
	-----------------------
*/
double CSP_metric_mae::score(void)
{
	return sum_absolute_error / predictions_made;
}

/*
	CSP_METRIC_MAE::RESET()
	-----------------------
*/
void CSP_metric_mae::reset(void)
{
	sum_absolute_error = 0;
	predictions_made = 0;
}
