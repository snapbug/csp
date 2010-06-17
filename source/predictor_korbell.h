/*
	PREDICTOR_KORBELL.H
	-------------------
*/

#include "predictor.h"

#ifndef PREDICTOR_KORBELL_H_
#define PREDICTOR_KORBELL_H_

class CSP_predictor_korbell : public CSP_predictor
{
public:
	CSP_predictor_korbell(CSP_dataset *dataset, uint64_t k, uint32_t *coraters);
	virtual ~CSP_predictor_korbell() {}
	
	virtual double predict(uint64_t user, uint64_t movie, uint64_t day);
	virtual void added_rating(uint64_t *key);
	virtual void removed_rating(uint64_t *key);

private:
	double global_average;
	double *user_effect, *movie_effect;
	uint64_t *user_counts, *movie_counts;
#ifdef TIME_EFFECTS
	uint64_t *user_first_ratings, *movie_first_ratings;
	double *user_time_user_effect, *user_time_user_bottom, *user_time_user_average;
	double *user_time_movie_effect, *user_time_movie_bottom, *user_time_movie_average;
	double *movie_time_movie_effect, *movie_time_movie_bottom, *movie_time_movie_average;
	double *movie_time_user_effect, *movie_time_user_bottom, *movie_time_user_average;
#endif
	double *user_movie_average_effect, *user_movie_average_bottom, *user_movie_average_average;
	double *movie_average;
	double *user_movie_support_effect, *user_movie_support_bottom, *user_movie_support_average;
	double *movie_user_average_effect, *movie_user_average_bottom, *movie_user_average_average;
	double *user_average;
	double *movie_user_support_effect, *movie_user_support_bottom, *movie_user_support_average;
	
	double movie_alpha, user_alpha, user_movie_average_alpha, user_movie_support_alpha, movie_user_average_alpha, movie_user_support_alpha;
#ifdef TIME_EFFECTS
	double user_time_user_alpha, user_time_movie_alpha, movie_time_movie_alpha, movie_time_user_alpha;
#endif
	
	uint32_t *coraters;
	uint64_t k;
	double beta;
	
	double predict_statistics(uint64_t user, uint64_t movie, uint64_t day);
	double predict_neighbour(uint64_t user, uint64_t movie, uint64_t day);
	static int neighbour_compare(const void *a, const void *b);
	void non_negative_quadratic_opt(float *a, float *b, double *w, uint64_t size);
	
	float *correlation;
	float *abar_tri, *abar_dia, *bbar;
	double bar_avg_tri_top, bar_avg_dia_top;
	uint64_t bar_avg_tri_bot, bar_avg_dia_bot;
	
	typedef struct {
		float correlation;
		uint32_t coraters;
		uint64_t movie_id;
		uint64_t considered;
		double residual;
	} neighbour;
	
};

#endif /* PREDICTOR_KORBELL_H_ */
