/*
	PREDICTOR_USER_KNN.C
	--------------------
*/

#include <stdlib.h>
#include <stdio.h>
#include "predictor_user_knn.h"

/*
	CSP_PREDICTOR_USER_KNN::CSP_PREDICTOR_USER_KNN()
	------------------------------------------------
*/
CSP_predictor_user_knn::CSP_predictor_user_knn(CSP_dataset *dataset, uint64_t k) : CSP_predictor(dataset), k(k)
{
	if (!dataset->loaded_extra)
		exit(puts("Need to have loaded extra data (-e) to use User KNN"));
}

/*
	CSP_PREDICTOR_USER_KNN::PREDICT()
	---------------------------------
*/
double CSP_predictor_user_knn::predict(uint64_t user, uint64_t movie, uint64_t day)
{
	printf("Predicting %lu %lu %lu\n", user, movie, day);
	return 3.0;
}

/*
	CSP_PREDICTOR_USER_KNN::ADDED_RATING()
	--------------------------------------
*/
void CSP_predictor_user_knn::added_rating(uint64_t *key)
{
	key = key;
}

/*
	CSP_PREDICTOR_USER_KNN::REMOVED_RATING()
	----------------------------------------
*/
void CSP_predictor_user_knn::removed_rating(uint64_t *key)
{
	key = key;
}
