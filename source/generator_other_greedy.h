/*
	GENERATOR_OTHER_GREEDY.H
	------------------------
*/
#include "generator_greedy_cheat.h"
#include "predictor.h"
#include "metric.h"

#define NUMCONSIDER 5

#ifndef GENERATOR_OTHER_GREEDY_H_
#define GENERATOR_OTHER_GREEDY_H_

static uint64_t number_times_start[] =
#include "init.greedy.dat"

class CSP_generator_other_greedy : public CSP_generator_greedy_cheat
{
public:
	CSP_generator_other_greedy(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric);
	virtual ~CSP_generator_other_greedy() {}

	virtual uint64_t next_movie(uint64_t user, uint64_t which_one, uint64_t *key);
	
private:
	typedef struct {
		uint64_t movie_id;
		uint64_t number_times;
	} movie;
	movie *number_times_greedy;
	
	static int number_times_cmp(const void *a, const void *b);
	static int movie_id_search(const void *a, const void *b);

};

#endif /* GENERATOR_OTHER_GREEDY_H_ */

