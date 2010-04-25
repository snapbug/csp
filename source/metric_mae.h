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
	CSP_metric_mae(CSP_dataset *dataset, CSP_predictor *predictor);
	virtual ~CSP_metric_mae() {}

	virtual double score(uint64_t user);
//	virtual void update(double predicted, double actual);
//	virtual void reset(void);

//private:
//	double sum_absolute_error;
//	uint64_t predictions_made;
};

#endif /* METRIC_MAE_H_ */
