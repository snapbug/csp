/*
	GENERATOR_NAIVE_BAYES.H
	-----------------------
*/
#include "generator_entropy.h"

#ifndef GENERATOR_NAIVE_BAYES_H_
#define GENERATOR_NAIVE_BAYES_H_

class CSP_generator_naive_bayes : public CSP_generator_entropy
{
public:
	CSP_generator_naive_bayes(CSP_dataset *dataset, uint32_t *coraters);
	virtual ~CSP_generator_naive_bayes() {}

	virtual uint64_t *generate(uint64_t user, uint64_t number_presented);

private:
	static int probability_cmp(const void *a, const void *b);
	double calculate_probability(uint64_t movie, uint64_t non_ratable, uint64_t ratable);
	
	uint32_t *coraters;
	uint64_t last_presented_and_seen;
};

#endif /* GENERATOR_NAIVE_BAYES_H_ */

