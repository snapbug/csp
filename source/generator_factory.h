/*
	GENERATOR_FACTORY.H
	-------------------
*/

#include "generator.h"
#include "generator_greedy_cheat.h"
#include "generator_random.h"
#include "generator_entropy.h"
#include "generator_popularity.h"
#include "generator_item_avg.h"
#include "generator_naive_bayes.h"

#ifndef GENERATOR_FACTORY_H_
#define GENERATOR_FACTORY_H_

class CSP_generator_factory
{
public:
	enum {
		NONE,
		ITEM_AVERAGE,
		RANDOM,
		POPULARITY,
		BAYESIAN,
		ENTROPY,
		GREEDY_CHEAT
	};
};

#endif /* GENERATOR_FACTORY_H_ */
