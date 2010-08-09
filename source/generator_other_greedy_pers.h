/*
	GENERATOR_OTHER_GREEDY_PERS.H
	-----------------------------
*/
#include "generator_other_greedy.h"
#include "predictor.h"
#include "metric.h"

#ifndef GENERATOR_OTHER_GREEDY_PERS_H_
#define GENERATOR_OTHER_GREEDY_PERS_H_

class CSP_generator_other_greedy_pers : public CSP_generator_other_greedy
{
public:
	CSP_generator_other_greedy_pers(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric);
	virtual ~CSP_generator_other_greedy_pers() {}

	virtual uint64_t next_movie(uint64_t user, uint64_t which_one, uint64_t *key);
	
private:
	CSP_metric *metric;
	CSP_predictor *predictor;

};

#endif /* GENERATOR_OTHER_GREEDY_H_ */

