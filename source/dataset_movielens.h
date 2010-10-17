/*
	DATASET_MOVIELENS.H
	-----------------
*/
#include <stdio.h>
#include "dataset.h"

#ifndef DATASET_MOVIELENS_H_
#define DATASET_MOVIELENS_H_

class CSP_dataset_movielens : public CSP_dataset
{
public:
	CSP_dataset_movielens(CSP_param_block *params);
	virtual ~CSP_dataset_movielens() {}
	
	virtual uint64_t *ratings_for_user(uint64_t user, uint64_t *count);
	virtual uint64_t *test_ratings_for_user(uint64_t user, uint64_t *count);
	virtual uint64_t *ratings_for_movie(uint64_t movie, uint64_t *count);
	virtual uint64_t *get_ratings(uint64_t *count);
	virtual uint64_t *get_test_ratings(uint64_t *count);
	
	inline virtual void add_rating(uint64_t *rating) { *rating = *rating | (1ULL << 49ULL); }
	inline virtual void remove_rating(uint64_t *rating) { *rating = *rating & ~(1ULL << 49ULL); }
	
	/*
		Functions to extract the data from the actual items
	*/
	inline virtual uint64_t included(uint64_t example) { return example >> 49 & 1; }
	inline virtual uint64_t user(uint64_t example) { return example >> 18 & 262143; }
	inline virtual uint64_t movie(uint64_t example) { return example >> 4 & 16383; }
	inline virtual uint64_t day(uint64_t example) { return example - example; }
	inline virtual uint64_t rating(uint64_t example) { return example >> 0 & 31; }
	
private:
	uint64_t data[9029516];
	uint64_t index[69878];
	uint64_t testing_data[970538];
	uint64_t testing_index[69878];
	uint64_t extra_data[9029516];
	uint64_t extra_index[10411];
};

#endif /* DATASET_MOVIELENS_H_ */
