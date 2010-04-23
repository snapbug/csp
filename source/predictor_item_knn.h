/*
	PREDICTOR_ITEM_KNN.H
	--------------------
*/

#include "predictor.h"

#ifndef PREDICTOR_ITEM_KNN_H_
#define PREDICTOR_ITEM_KNN_H_

#define CORR(i, j) (((i * (35539 - i)) / 2) + j - i - 1)

class CSP_predictor_item_knn : public CSP_predictor
{
public:
	CSP_predictor_item_knn(CSP_dataset *dataset, uint64_t k);
	virtual ~CSP_predictor_item_knn() {}
	
	virtual double predict(uint64_t user, uint64_t movie, uint64_t day);
	virtual void added_rating(uint64_t *key);
	virtual void removed_rating(uint64_t *key);

	static int similarity_cmp(const void *a, const void *b);

private:
	typedef struct {
		double x;       // sum of ratings for movie X
		double y;       // sum of ratings for movie Y
		double xx;      // squared sum of ratings for movie X
		double yy;      // squared sum of ratings for movie Y
		double xy;      // product of ratings for movie X and Y
		uint64_t count; // number of co-raters
	} pearson_intermediate;
	pearson_intermediate *intermediate;

	typedef struct {
		double correlation;
		uint64_t movie_id;
		uint64_t rating; // the rating the user gave this other movie
		uint64_t considered; // whether we need to consider this is calculation (used as primary sort index)
	} movie;
	movie *most_similar;
	
	uint64_t k;
};

#endif /* PREDICTOR_ITEM_KNN_H_ */
