/*
	PREDICTOR_USER_AVG.H
	--------------------
*/

#include "predictor.h"

#ifndef PREDICTOR_USER_AVG_H_
#define PREDICTOR_USER_AVG_H_

class CSP_predictor_user_avg : public CSP_predictor
{
public:
	CSP_predictor_user_avg(CSP_dataset *dataset);
	virtual ~CSP_predictor_user_avg() {}

	virtual double predict(uint64_t user, uint64_t movie, uint64_t day);
	virtual void added_rating(uint64_t *key);
	virtual void removed_rating(uint64_t *key);

private:
	double *top;
	uint64_t *bot;
};

#endif /* PREDICTOR_USER_AVG_H_ */
