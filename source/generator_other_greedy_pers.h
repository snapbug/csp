/*
	GENERATOR_OTHER_GREEDY_PERS.H
	-----------------------------
*/
#include "generator_other_greedy.h"
#include "generator_naive_bayes.h"
#include "predictor.h"
#include "metric.h"

#ifndef GENERATOR_OTHER_GREEDY_PERS_H_
#define GENERATOR_OTHER_GREEDY_PERS_H_

#ifndef PERTURB
	#define PERTURB 2 
#endif

class CSP_generator_other_greedy_pers : public CSP_generator_other_greedy
{
public:
	CSP_generator_other_greedy_pers(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric, uint32_t *coraters);
	virtual ~CSP_generator_other_greedy_pers() {}

	virtual uint64_t next_movie(uint64_t user, uint64_t which_one, uint64_t *key);
	
private:
	typedef struct {
		uint64_t movie_id;
		uint64_t number_times;
		double top;
		double bot;
	} movie;
	movie *most_greedy_prob;
	
	static int number_times_cmp(const void *a, const void *b);
	static int probability_cmp(const void *a, const void *b);
	double calculate_probability(uint64_t movie, uint64_t other, uint64_t *key);
	
	uint32_t *coraters;

};

#endif /* GENERATOR_OTHER_GREEDY_H_ */
