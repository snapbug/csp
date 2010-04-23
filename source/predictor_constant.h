/*
	PREDICTOR_CONSTANT.H
	--------------------
*/

#include "predictor.h"

#ifndef PREDICTOR_CONSTANT_H_
#define PREDICTOR_CONSTANT_H_

class CSP_predictor_constant : public CSP_predictor
{
public:
	CSP_predictor_constant(CSP_dataset *dataset);
	virtual ~CSP_predictor_constant() {}

	virtual double predict(uint64_t user, uint64_t movie, uint64_t day);

private:
	double constant;
};

#endif /* PREDICTOR_CONSTANT_H_ */
