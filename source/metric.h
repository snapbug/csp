/*
	METRIC.H
	--------
*/

#include "csp_types.h"
#include "dataset.h"
#include "predictor.h"

#ifndef METRIC_H_
#define METRIC_H_

class CSP_metric
{
public:
	CSP_metric(CSP_dataset * dataset, CSP_predictor *predictor) : dataset(dataset), predictor(predictor) {}
	virtual ~CSP_metric() {}

	virtual double score(uint64_t user) = 0;
	//virtual void update(double predicted, double actual) = 0;
	//virtual void reset(void) = 0;

	virtual void set_limits(uint64_t high, uint64_t low) { high = high; low = low; }

protected:
	CSP_dataset *dataset;
	CSP_predictor *predictor;

};

#endif /* METRIC_H_ */
