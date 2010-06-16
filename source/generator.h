/*
	GENERATOR.H
	-----------
*/
#include "csp_types.h"
#include "dataset.h"

#ifndef GENERATOR_H_
#define GENERATOR_H_

class CSP_generator
{
public:
	CSP_generator(CSP_dataset *dataset) : dataset(dataset) {}
	virtual ~CSP_generator() {}

	virtual void generate(uint64_t user, uint64_t *presentation_list, uint64_t number_presented) = 0;

protected:
	CSP_dataset *dataset;
};

#endif /* GENERATOR_H_ */

