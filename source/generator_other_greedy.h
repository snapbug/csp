/*
	GENERATOR_OTHER_GREEDY.H
	------------------------
*/
#include "generator_greedy_cheat.h"
#include "predictor.h"
#include "metric.h"

#ifndef GENERATOR_OTHER_GREEDY_H_
#define GENERATOR_OTHER_GREEDY_H_

class CSP_generator_other_greedy : public CSP_generator_greedy_cheat
{
public:
	CSP_generator_other_greedy(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric);
	virtual ~CSP_generator_other_greedy() {}

	virtual uint64_t *generate(uint64_t user, uint64_t number_presented);
	
private:
	typedef struct {
		uint64_t movie_id;
		uint64_t number_times;
	} movie;
	movie *number_times_greedy;
	
	CSP_metric *metric;
	CSP_predictor *predictor;
	
	static int movie_id_cmp(const void *a, const void *b);
	static int number_times_cmp(const void *a, const void *b);

};

#endif /* GENERATOR_OTHER_GREEDY_H_ */

