/*
	METRIC.H
	--------
*/

#include "csp_types.h"

#ifndef METRIC_H_
#define METRIC_H_

class CSP_metric
{
public:
	CSP_metric() {}
	virtual ~CSP_metric() {}

	virtual void update(double predicted, double actual) = 0;
	virtual double score(void) = 0;
	virtual void reset(void) = 0;

	virtual void set_limits(uint64_t high, uint64_t low) { high = high; low = low; }

};

#endif /* METRIC_H_ */
