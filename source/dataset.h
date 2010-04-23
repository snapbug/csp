/*
	DATASET.H
	---------
*/

#include "csp_types.h"
#include "param_block.h"

#ifndef DATASET_H_
#define DATASET_H_

class CSP_dataset
{
public:
	CSP_dataset(CSP_param_block *params) : params(params) { loaded_extra = FALSE; }
	virtual ~CSP_dataset() {}

	uint64_t number_users;
	uint64_t number_items;
	uint64_t number_ratings;
	uint64_t number_test_ratings;
	uint64_t loaded_extra; // whether we loaded the extra data -- useful for predictors to know, can make some faster if we have

	uint64_t minimum, maximum; // min/max allowed rating values

	virtual uint64_t *ratings_for_user(uint64_t user, uint64_t *count) = 0;
	virtual uint64_t *test_ratings_for_user(uint64_t user, uint64_t *count) = 0;
	virtual uint64_t *ratings_for_movie(uint64_t movie, uint64_t *count) = 0;
	virtual uint64_t *get_ratings(uint64_t *count) = 0;
	virtual uint64_t *get_test_ratings(uint64_t *count) = 0;
	
	inline virtual void add_rating(uint64_t *rating) { rating = rating; }
	inline virtual void remove_rating(uint64_t *rating) { rating = rating; }

	/*
		Functions to extract values from each example in the datset
	*/
	inline virtual uint64_t included(uint64_t example) { example = example; return 0; }
	inline virtual uint64_t user(uint64_t example) { example = example; return 0; }
	inline virtual uint64_t movie(uint64_t example) { example = example; return 0; }
	inline virtual uint64_t day(uint64_t example) { example = example; return 0; }
	inline virtual uint64_t rating(uint64_t example) { example = example; return 0; }
	inline virtual uint64_t included(uint64_t *example) { return included(*example); }
	inline virtual uint64_t user(uint64_t *example) { return user(*example); }
	inline virtual uint64_t movie(uint64_t *example) { return movie(*example); }
	inline virtual uint64_t day(uint64_t *example) { return day(*example); }
	inline virtual uint64_t rating(uint64_t *example) { return rating(*example); }

protected:
	CSP_param_block *params;
};

#endif /* DATASET_H_ */
