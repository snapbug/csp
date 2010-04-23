/*
	PREDICTOR_USER_AVG.C
	--------------------
*/
#include <stdio.h>
#include "predictor_user_avg.h"

/*
	CSP_PREDICTOR_USER_AVG::CSP_PREDICTOR_USER_AVG()
	------------------------------------------------
*/
CSP_predictor_user_avg::CSP_predictor_user_avg(CSP_dataset *dataset) : CSP_predictor(dataset)
{
	uint64_t i, j;
	uint64_t *user_ratings;
	
	top = new double[dataset->number_users];
	bot = new uint64_t[dataset->number_users];

	for (i = 0; i < dataset->number_users; i++)
	{
		top[i] = 0;
		user_ratings = dataset->ratings_for_user(i, &bot[i]);
		for (j = 0; j < bot[i]; j++)
			top[i] += (double)dataset->rating(user_ratings[j]);
	}
}

/*
	CSP_PREDICTOR_USER_AVG::ADDED_RATING()
	--------------------------------------
*/
void CSP_predictor_user_avg::added_rating(uint64_t *key)
{
	top[dataset->user(key)] += dataset->rating(key);
	bot[dataset->user(key)]++;
}

/*
	CSP_PREDICTOR_USER_AVG::REMOVED_RATING()
	----------------------------------------
*/
void CSP_predictor_user_avg::removed_rating(uint64_t *key)
{
	top[dataset->user(key)] -= dataset->rating(key);
	bot[dataset->user(key)]--;
}

/*
	CSP_PREDICTOR_USER_AVG::PREDICT()
	---------------------------------
*/
double CSP_predictor_user_avg::predict(uint64_t user, uint64_t movie, uint64_t day)
{
	movie = movie;
	day = day;
	
	if (bot[user] == 0)
		return 3.6;
	return top[user] / bot[user];
}
