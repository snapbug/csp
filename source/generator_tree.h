/*
	GENERATOR_TREE.H
	----------------
*/
#include "generator_greedy_cheat.h"

#ifndef GENERATOR_TREE_H_
#define GENERATOR_TREE_H_

class CSP_generator_tree : public CSP_generator_greedy_cheat
{
public:
	CSP_generator_tree(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric);
	virtual ~CSP_generator_tree() {}
	
	virtual uint64_t next_movie(uint64_t user, uint64_t which_one, uint64_t *key);

private:
	static int number_times_cmp(const void *a, const void *b);
	static int movie_id_cmp(const void *a, const void *b);

	inline int parity(uint64_t rating);
	
	typedef struct {
		uint64_t number_times; // the number of times in other peoples greedy
		uint64_t number_seen; // the number of times seen by other people
		uint64_t movie_id; // the movie id
		uint64_t included; // whether we're counting it or not
	} movie;
	movie *most_greedy;
	uint64_t *users;
	uint64_t *history;
	uint64_t history_len;
	uint32_t *coraters;
	
};

#endif /* GENERATOR_TREE_H_ */

