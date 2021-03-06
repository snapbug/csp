/*
	GENERATOR_RANDOM.H
	------------------
*/
#include "generator.h"

#ifndef GENERATOR_RANDOM_H_
#define GENERATOR_RANDOM_H_

class CSP_generator_random : public CSP_generator
{
public:
	CSP_generator_random(CSP_dataset *dataset);
	virtual ~CSP_generator_random() {}

	virtual uint64_t next_movie(uint64_t user, uint64_t which_one, uint64_t *key);

private:
	void shuffle(uint64_t *start, uint64_t number);
	uint64_t *mids;

};

#endif /* GENERATOR_RANDOM_H_ */

