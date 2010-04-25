/*
	METRIC_RMSE.H
	-------------
*/
#include "metric.h"

#ifndef METRIC_RMSE_H_
#define METRIC_RMSE_H_

class CSP_metric_rmse : public CSP_metric
{
public:
	CSP_metric_rmse(CSP_dataset *dataset, CSP_predictor *predictor);
	virtual ~CSP_metric_rmse() {}

	virtual double score(uint64_t user);
	//virtual void update(double predicted, double actual);
	//virtual void reset(void);

//private:
//	double sum_sqaured_error;
//	uint64_t predictions_made;
};

#endif /* METRIC_RMSE_H_ */
