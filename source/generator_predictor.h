/*
	GENERATOR_PREDICTOR.H
	---------------------
*/
#include "predictor.h"
#include "metric.h"
#include "generator.h"

#ifndef GENERATOR_PREDICTOR_H_
#define GENERATOR_PREDICTOR_H_

class CSP_generator_predictor : public CSP_generator
{
public:
	CSP_generator_predictor(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric);
	virtual ~CSP_generator_predictor() {}
	
	virtual uint64_t next_movie(uint64_t user, uint64_t which_one, uint64_t *key);

protected:
	CSP_metric *metric;
	CSP_predictor *predictor;
	
	uint64_t NUMCONSIDER;
	
private:
	typedef struct {
		uint64_t movie_id;
		double prediction;
	} movie;
	static int error_cmp(const void *a, const void *b);
	movie *error_reduction;
	
};

#endif /* GENERATOR_TREE_H_ */

