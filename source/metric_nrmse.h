/*
	METRIC_NRMSE.H
	--------------
*/
#include "metric_rmse.h"

#ifndef METRIC_NRMSE_H_
#define METRIC_NRMSE_H_

class CSP_metric_nrmse : public CSP_metric_rmse
{
public:
	CSP_metric_nrmse(CSP_dataset *dataset, CSP_predictor *predictor);
	virtual ~CSP_metric_nrmse() {}

	virtual double score(uint64_t user);
	//virtual void update(double predicted, double actual);
	//virtual void reset(void);
	//virtual void set_limits(uint64_t high, uint64_t low);

//private:
//	double sum_sqaured_error;
//	uint64_t high, low;
//	uint64_t predictions_made;
};

#endif /* METRIC_NRMSE_H_ */
