/*
	METRIC_NMAE.H
	------------
*/
#include "metric_mae.h"

#ifndef METRIC_NMAE_H_
#define METRIC_NMAE_H_

class CSP_metric_nmae : public CSP_metric_mae
{
public:
	CSP_metric_nmae(CSP_dataset *dataset, CSP_predictor *predictor);
	virtual ~CSP_metric_nmae() {}

	//virtual void update(double predicted, double actual);
	//virtual void reset(void);
	virtual double score(uint64_t user);
	//virtual void set_limits(uint64_t high, uint64_t low);

//private:
//	double sum_absolute_error;
//	uint64_t high, low;
//	uint64_t predictions_made;
};

#endif /* METRIC_NMAE_H_ */
