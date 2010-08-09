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

	virtual uint64_t next_movie(uint64_t user, uint64_t which_one, uint64_t *key);

private:
	static int probability_cmp(const void *a, const void *b);
	double calculate_probability(uint64_t movie, uint64_t other, uint64_t *key);
	
	uint32_t *coraters;
	uint64_t last_presented_and_seen;
	
	typedef struct {
		uint64_t movie_id;
		double probability;
	} movie;
	
	movie *most_probable;
};

#endif /* GENERATOR_NAIVE_BAYES_H_ */

