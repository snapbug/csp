/*
	PREDICTOR_ITEM_AVG.H
	--------------------
*/

#include "predictor.h"

#ifndef PREDICTOR_ITEM_AVG_H_
#define PREDICTOR_ITEM_AVG_H_

class CSP_predictor_item_avg : public CSP_predictor
{
public:
	CSP_predictor_item_avg(CSP_dataset *dataset);
	virtual ~CSP_predictor_item_avg() {}

	virtual double predict(uint64_t user, uint64_t movie, uint64_t day);

private:
	double *averages;
};

#endif /* PREDICTOR_ITEM_AVG_H_ */
