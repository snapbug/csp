/*
	PREDICTOR_CONSTANT.C
	--------------------
*/

#include "predictor_constant.h"

CSP_predictor_constant::CSP_predictor_constant(CSP_dataset *dataset) : CSP_predictor(dataset)
{
	constant = 3.0;
}

double CSP_predictor_constant::predict(uint64_t user, uint64_t movie, uint64_t day)
{
	user = user;
	movie = movie;
	day = day;
	return constant;
}
