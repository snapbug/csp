/*
	PREDICTOR_RANDOM.H
	------------------
*/

#include "predictor.h"

#ifndef PREDICTOR_RANDOM_H_
#define PREDICTOR_RANDOM_H_

class CSP_predictor_random : public CSP_predictor
{
public:
	CSP_predictor_random(CSP_dataset *dataset) : CSP_predictor(dataset) {}
	virtual ~CSP_predictor_random() {}

	virtual double predict(uint64_t user, uint64_t movie, uint64_t day);
};

#endif /* PREDICTOR_RANDOM_H_ */
