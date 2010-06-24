/*
	DATASET_NETFLIX.H
	-----------------
*/
#include <stdio.h>
#include "dataset.h"

#ifndef DATASET_NETFLIX_H_
#define DATASET_NETFLIX_H_

class CSP_dataset_netflix : public CSP_dataset
{
public:
	CSP_dataset_netflix(CSP_param_block *params);
	virtual ~CSP_dataset_netflix() {}
	
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
	inline virtual uint64_t user(uint64_t example) { return example >> 30 & 524287; }
	inline virtual uint64_t movie(uint64_t example) { return example >> 15 & 32767; }
	inline virtual uint64_t day(uint64_t example) { return example >> 3 & 4095; }
	inline virtual uint64_t rating(uint64_t example) { return example >> 0 & 7; }
	
private:
	uint64_t data[100480507]; // this is the most possible ratings we could have, won't use all this space
	uint64_t index[480189];
	uint64_t testing_data[10183402]; // the largest test set is the proportional one
	uint64_t testing_index[480189];
	uint64_t extra_data[100480507];
	uint64_t extra_index[480189];
};

#endif /* DATASET_NETFLIX_H_ */
