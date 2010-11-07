/*
	GENERATOR_GREEDY_CHEAT.H
	------------------------
*/
#include "generator.h"
#include "predictor.h"
#include "metric.h"

#ifndef GENERATOR_GREEDY_CHEAT_H_
#define GENERATOR_GREEDY_CHEAT_H_

#ifndef NUMDONE
	#ifdef ML
		#define NUMDONE 10
	#else
		#define NUMDONE 11
	#endif
#endif

static uint64_t greedy_movies[] = {
#ifdef ML
	#include "greedy.mae.ml.txt"
#else
	#ifndef MAE
		#warning "Using RMSE"
		#include "greedy.rmse.nf.txt"
	#else
		#warning "Using MAE"
		#include "greedy.mae.nf.txt"
	#endif
#endif
};

class CSP_generator_greedy_cheat : public CSP_generator
{
public:
	CSP_generator_greedy_cheat(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric);
	virtual ~CSP_generator_greedy_cheat() {}

	virtual uint64_t next_movie(uint64_t user, uint64_t which_one, uint64_t *key);
	
protected:
	CSP_metric *metric;
	CSP_predictor *predictor;
	
	uint64_t NUMCONSIDER;
	
private:
	typedef struct {
		uint64_t movie_id;
		double prediction_error;
	} movie;
	static int error_cmp(const void *a, const void *b);
	movie *error_reduction;
};

#endif /* GENERATOR_GREEDY_CHEAT_H_ */

