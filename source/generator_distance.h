/*
	GENERATOR_DISTANCE.H
	--------------------
*/
#include "generator.h"

#ifndef GENERATOR_DISTANCE_H_
#define GENERATOR_DISTANCE_H_

class CSP_generator_distance : public CSP_generator
{
public:
	CSP_generator_distance(CSP_dataset *dataset);
	virtual ~CSP_generator_distance() {}

	virtual uint64_t next_movie(uint64_t user, uint64_t which_one, uint64_t *key);

private:
	static int distance_cmp(const void *a, const void *b);
	
	typedef struct {
		double distance;
		uint64_t movie_id;
	} movie;
	movie *most_distant;
};

#endif /* GENERATOR_DISTANCE_H_ */

