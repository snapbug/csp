/*
	PREDICTOR_USER_KNN.H
	--------------------
*/

#include "predictor.h"

#ifndef PREDICTOR_USER_KNN_H_
#define PREDICTOR_USER_KNN_H_

class CSP_predictor_user_knn : public CSP_predictor
{
public:
	CSP_predictor_user_knn(CSP_dataset *dataset, uint64_t k);
	virtual ~CSP_predictor_user_knn() {}
	
	virtual double predict(uint64_t user, uint64_t movie, uint64_t day);
	virtual void added_rating(uint64_t *key);
	virtual void removed_rating(uint64_t *key);

private:
	uint64_t k;
};

#endif /* PREDICTOR_USER_KNN_H_ */
