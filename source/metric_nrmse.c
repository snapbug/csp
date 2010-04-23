/*
	METRIC_NRMSE.C
	--------------
*/

#include <math.h>
#include "metric_nrmse.h"

/*
	CSP_METRIC_NRMSE::CSP_METRIC_NRMSE()
	------------------------------------
*/
CSP_metric_nrmse::CSP_metric_nrmse() : CSP_metric()
{
	predictions_made = 0;
	sum_sqaured_error = 0;
}

/*
	CSP_METRIC_NRMSE::UPDATE()
	--------------------------
*/
void CSP_metric_nrmse::update(double predicted, double actual)
{
	sum_sqaured_error += pow(predicted - actual, 2);
	predictions_made++;
}

/*
	CSP_METRIC_NRMSE::SCORE()
	-------------------------
*/
double CSP_metric_nrmse::score(void)
{
	return sqrt(sum_sqaured_error / predictions_made) / (high - low);
}

/*
	CSP_METRIC_NRMSE::RESET()
	-------------------------
*/
void CSP_metric_nrmse::reset(void)
{
	sum_sqaured_error = 0;
	predictions_made = 0;
}

/*
	CSP_METRIC_NRMSE::SET_LIMITS()
	------------------------------
*/
void CSP_metric_nrmse::set_limits(uint64_t high, uint64_t low)
{
	this->high = high;
	this->low = low;
}
