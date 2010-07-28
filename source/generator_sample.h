/*
	GENERATOR_SAMPLE.H
	------------------
*/
#include "generator.h"
#include "predictor.h"
#include "metric.h"

#ifndef GENERATOR_SAMPLE_H_
#define GENERATOR_SAMPLE_H_

class CSP_generator_sample : public CSP_generator
{
public:
	CSP_generator_sample(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric);
	virtual ~CSP_generator_sample() {}

	virtual uint64_t *generate(uint64_t user, uint64_t number_presented);

private:
	static int error_cmp(const void *a, const void *b);
	static int count_cmp(const void *a, const void *b);
	
	CSP_metric *metric;
	CSP_predictor *predictor;
	
	typedef struct {
		uint64_t movie_id;
		double prediction_error;
	} error_movie;
	error_movie *error_reduction;
	
	typedef struct {
		uint64_t movie_id;
		uint64_t count;
	} count_movie;
	count_movie *first_positions;

};

#endif /* GENERATOR_SAMPLE_H_ */

