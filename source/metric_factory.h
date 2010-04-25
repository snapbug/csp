/*
	METRIC_FACTORY.H
	----------------
*/

#include "metric.h"

#ifndef METRIC_FACTORY_H_
#define METRIC_FACTORY_H_

class CSP_metric_factory : public CSP_metric
{
public:
	CSP_metric_factory(CSP_dataset *dataset, CSP_predictor *predictor);
	virtual ~CSP_metric_factory() {}

	//virtual void update(double predicted, double actual);
	virtual double score(uint64_t user);
	//virtual void reset(void);
	virtual void set_limits(uint64_t high, uint64_t low);
	
	void print(uint64_t user, uint64_t added);
	void set_metrics(uint64_t which) { metrics_to_use = which; }

	enum
	{
		NONE    = 0,
		MAE     = 1,
		NMAE    = 2,
		RMSE    = 4,
		NRMSE   = 8,
		ROC     = 16,
		CROC    = 32,
		UTILITY = 64
	};

private:
	uint64_t metrics_to_use;
};

#endif /* METRIC_FACTORY_H_ */
