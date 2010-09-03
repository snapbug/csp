/*
	GENERATOR_TREE.H
	----------------
*/
#include "generator_greedy_cheat.h"

#ifndef GENERATOR_TREE_H_
#define GENERATOR_TREE_H_

static uint64_t number_times[] =
#include "init.greedy.dat"
#ifndef NUMCONSIDER
	#define NUMCONSIDER 5
#endif

class CSP_generator_tree : public CSP_generator_greedy_cheat
{
public:
	CSP_generator_tree(CSP_dataset *dataset, CSP_predictor *predictor, CSP_metric *metric);
	virtual ~CSP_generator_tree() {}
	
	virtual uint64_t next_movie(uint64_t user, uint64_t which_one, uint64_t *key);

private:
	static int number_times_cmp(const void *a, const void *b);
	static int movie_id_cmp(const void *a, const void *b);
	static int movie_user_search(const void *a, const void *b);
	static int movie_greedy_search(const void *a, const void *b);
	
	typedef struct {
		uint64_t number_times;
		uint64_t movie_id;
	} movie;
	movie *most_greedy;
	uint64_t *users;
	

};

#endif /* GENERATOR_TREE_H_ */

