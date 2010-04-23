/*
	PREDICTOR.H
	-----------
*/

#include "csp_types.h"
#include "dataset.h"

#ifndef PREDICTOR_H_
#define PREDICTOR_H_

class CSP_predictor
{
public:
	CSP_predictor(CSP_dataset *dataset) : dataset(dataset) {}
	CSP_predictor(CSP_dataset *dataset, uint64_t k) : dataset(dataset) { k = k; }
	virtual ~CSP_predictor() {}

	virtual double predict(uint64_t user, uint64_t movie, uint64_t day) = 0;
	virtual void added_rating(uint64_t *key) { key = key; }
	virtual void removed_rating(uint64_t *key) { key = key; }

protected:
	CSP_dataset *dataset;
};

#endif /* PREDICTOR_H_ */
