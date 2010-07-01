/*
	GENERATOR_GREEDY_CHEAT.C
	------------------------
*/
#include "generator_greedy_cheat.h"

/*
	CSP_GENERATOR_GREEDY_CHEAT::CSP_GENERATOR_GREEDY_CHEAT()
	--------------------------------------------------------
*/
CSP_generator_greedy_cheat::CSP_generator_greedy_cheat(CSP_dataset *dataset, CSP_predictor *predictor) : CSP_generator(dataset), predictor(predictor)
{
}

/*
	CSP_GENERATOR_GREEDY_CHEAT::GENERATE()
	--------------------------------------
*/
uint64_t *CSP_generator_greedy_cheat::generate(uint64_t user, uint64_t number_presented)
{
	UNUSED(user);
	UNUSED(number_presented);
	
	return presentation_list;
}
