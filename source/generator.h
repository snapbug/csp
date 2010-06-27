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
	CSP_generator(CSP_dataset *dataset) : dataset(dataset) { presentation_list = new uint64_t[dataset->number_items]; }
	virtual ~CSP_generator() {}
	
	virtual uint64_t *generate(uint64_t user, uint64_t number_presented) = 0;
	
protected:
	CSP_dataset *dataset;
	uint64_t *presentation_list;
};

#endif /* GENERATOR_H_ */

