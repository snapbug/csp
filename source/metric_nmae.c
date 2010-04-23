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
CSP_metric_nmae::CSP_metric_nmae() : CSP_metric()
{
	predictions_made = 0;
	sum_absolute_error = 0;
}

/*
	CSP_METRIC_NMAE::UPDATE()
	-------------------------
*/
void CSP_metric_nmae::update(double predicted, double actual)
{
	sum_absolute_error += fabs(predicted - actual);
	predictions_made++;
}

/*
	CSP_METRIC_NMAE::SCORE()
	------------------------
*/
double CSP_metric_nmae::score(void)
{
	return (sum_absolute_error / predictions_made) / (high - low);
}

/*
	CSP_METRIC_NMAE::RESET()
	------------------------
*/
void CSP_metric_nmae::reset(void)
{
	sum_absolute_error = 0;
	predictions_made = 0;
}

/*
	CSP_METRIC_NMAE::SET_LIMITS()
	-----------------------------
*/
void CSP_metric_nmae::set_limits(uint64_t high, uint64_t low)
{
	this->high = high;
	this->low = low;
}
