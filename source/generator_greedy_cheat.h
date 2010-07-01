/*
	GENERATOR_GREEDY_CHEAT.H
	------------------------
*/
#include "generator.h"
#include "predictor.h"

#ifndef GENERATOR_GREEDY_CHEAT_H_
#define GENERATOR_GREEDY_CHEAT_H_

class CSP_generator_greedy_cheat : public CSP_generator
{
public:
	CSP_generator_greedy_cheat(CSP_dataset *dataset) : CSP_generator(dataset) {}
	CSP_generator_greedy_cheat(CSP_dataset *dataset, CSP_predictor *predictor);
	virtual ~CSP_generator_greedy_cheat() {}

	virtual uint64_t *generate(uint64_t user, uint64_t number_presented);

private:
	CSP_predictor *predictor;

};

#endif /* GENERATOR_GREEDY_CHEAT_H_ */

