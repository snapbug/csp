/*
	PREDICTOR_FACTORY.H
	-------------------
*/

#include "predictor.h"
#include "predictor_random.h"
#include "predictor_global_avg.h"
#include "predictor_constant.h"
#include "predictor_item_knn.h"
#include "predictor_item_avg.h"
#include "predictor_user_avg.h"
#include "predictor_user_knn.h"
#include "predictor_korbell.h"

#ifndef PREDICTOR_FACTORY_H_
#define PREDICTOR_FACTORY_H_

class CSP_predictor_factory
{
public:
	enum {
		NONE,
		RANDOM,
		ITEM_AVERAGE,
		USER_AVERAGE,
		ITEM_ITEM_KNN,
		USER_USER_KNN,
		GLOBAL_AVERAGE,
		CONSTANT,
		KORBELL
	};
};

#endif /* PREDICTOR_FACTORY_H_ */
