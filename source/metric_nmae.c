/*
	METRIC_NMAE.C
	-------------
*/

#include <math.h>
#include "metric_nmae.h"

/*
	CSP_METRIC_NMAE::CSP_METRIC_NMAE()
	----------------------------------
*/
CSP_metric_nmae::CSP_metric_nmae(CSP_dataset *dataset, CSP_predictor *predictor) : CSP_metric_mae(dataset, predictor)
{
//	predictions_made = 0;
//	sum_absolute_error = 0;
}

/*
	CSP_METRIC_NMAE::UPDATE()
	-------------------------
*/
//void CSP_metric_nmae::update(double predicted, double actual)
//{
//	sum_absolute_error += fabs(predicted - actual);
//	predictions_made++;
//}

/*
	CSP_METRIC_NMAE::SCORE()
	------------------------
*/
double CSP_metric_nmae::score(uint64_t user)
{
	return CSP_metric_mae::score(user) / (dataset->maximum - dataset->minimum);
}

/*
	CSP_METRIC_NMAE::RESET()
	------------------------
*/
//void CSP_metric_nmae::reset(void)
//{
//	sum_absolute_error = 0;
//	predictions_made = 0;
//}

/*
	CSP_METRIC_NMAE::SET_LIMITS()
	-----------------------------
*/
//void CSP_metric_nmae::set_limits(uint64_t high, uint64_t low)
//{
//	this->high = high;
//	this->low = low;
//}
