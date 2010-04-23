/*
	METRIC_MAE.H
	------------
*/
#include "metric.h"

#ifndef METRIC_MAE_H_
#define METRIC_MAE_H_

class CSP_metric_mae : public CSP_metric
{
public:
	CSP_metric_mae();
	virtual ~CSP_metric_mae() {}

	virtual void update(double predicted, double actual);
	virtual double score(void);
	virtual void reset(void);

private:
	double sum_absolute_error;
	uint64_t predictions_made;
};

#endif /* METRIC_MAE_H_ */
