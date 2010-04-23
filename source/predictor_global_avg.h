/*
	PREDICTOR_GLOBAL_AVG.H
	----------------------
*/

#include "predictor.h"

#ifndef PREDICTOR_GLOBAL_AVG_H_
#define PREDICTOR_GLOBAL_AVG_H_

class CSP_predictor_global_avg : public CSP_predictor
{
public:
	CSP_predictor_global_avg(CSP_dataset *dataset);
	virtual ~CSP_predictor_global_avg() {}

	virtual double predict(uint64_t user, uint64_t movie, uint64_t day);
	virtual void added_rating(uint64_t *key);
	virtual void removed_rating(uint64_t *key);

private:
	uint64_t number_ratings;
	double sum_of_ratings;
};

#endif /* PREDICTOR_GLOBAL_AVG_H_ */
