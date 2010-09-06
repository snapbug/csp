/*
	GENERATOR_GREEDY_CHEAT.H
	------------------------
*/
#include "generator.h"
#include "predictor.h"
#include "metric.h"

#ifndef GENERATOR_GREEDY_CHEAT_H_
#define GENERATOR_GREEDY_CHEAT_H_

#ifndef NUMCONSIDER
	#define NUMCONSIDER 5
#endif
#ifndef NUMDONE
	#define NUMDONE 5
#endif

static uint64_t greedy_movies[] = {
	#include "greedy.choices.txt"
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
	
private:
	typedef struct {
		uint64_t movie_id;
		double prediction_error;
	} movie;
	static int error_cmp(const void *a, const void *b);
	movie *error_reduction;
};

#endif /* GENERATOR_GREEDY_CHEAT_H_ */

