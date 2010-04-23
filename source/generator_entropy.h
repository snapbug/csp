/*
	GENERATOR_ENTROPY.H
	-------------------
*/
#include "generator.h"

#ifndef GENERATOR_ENTROPY_H_
#define GENERATOR_ENTROPY_H_

class CSP_generator_entropy : public CSP_generator
{
public:
	CSP_generator_entropy(CSP_dataset *dataset);
	virtual ~CSP_generator_entropy() {}

	virtual uint64_t *generate(uint64_t user, uint64_t number_presented);
	static int entropy_cmp(const void *a, const void *b);

private:
	typedef struct {
		double entropy;
		uint64_t ratings;
		uint64_t movie_id;
	} movie;
	movie *most_entropic;
};

#endif /* GENERATOR_ENTROPY_H_ */

