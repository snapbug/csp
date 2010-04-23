/*
	GENERATOR_NAIVE_BAYES.C
	-----------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "generator_naive_bayes.h"
#define CORR(i, j) (((i * (35539 - i)) / 2) + j - i - 1)

/*
	CSP_GENERATOR_NAIVE_BAYES::CSP_GENERATOR_NAIVE_BAYES()
	------------------------------------------------------
*/
CSP_generator_naive_bayes::CSP_generator_naive_bayes(CSP_dataset *dataset) : CSP_generator_entropy(dataset)
{
	uint64_t i, j, k, min, max;
	uint64_t *this_one, *that_one;
	uint64_t this_count, that_count;
	
	if (!dataset->loaded_extra)
		exit(puts("Must use -e to use Naive Bayes generation method."));
	
	dataset->get_ratings(&total_ratings);
	base_probability = 1.0 * total_ratings / (dataset->number_items * dataset->number_users);
	last_presented_and_seen = 0;
	most_probable = new movie[dataset->number_items];
	probabilities = new double[dataset->number_items];
	coraters = new uint32_t[dataset->number_items * dataset->number_items / 2];
	
	for (i = 0; i < dataset->number_items; i++)
	{
		most_probable[i].movie_id = i;
		most_probable[i].probability = probabilities[i] = base_probability;
	}
	
	for (i = 0; i < dataset->number_items * dataset->number_items / 2; i++)
		coraters[i] = 0;
	
	/*
		Precalculate the co-raters for all movie pairs.
	*/
	fprintf(stderr, "Precalculating co-ratings...\n");
	for (i = 0; i < dataset->number_items; i++)
	{
		if (i % 100 == 0) { fprintf(stderr, "\r%5lu", i); fflush(stderr); }
		this_one = dataset->ratings_for_movie(i, &this_count);
		for (j = 0; j < this_count; j++)
		{
			that_one = dataset->ratings_for_user(dataset->user(this_one[j]), &that_count);
			for (k = 0; k < that_count; k++)
			{
				if (dataset->movie(that_one[k]) != i)
				{
					min = MIN(i, dataset->movie(that_one[k]));
					max = MAX(i, dataset->movie(that_one[k]));
					coraters[CORR(min, max)]++;
				}
			}
		}
	}
	fprintf(stderr, "\rDone.\n");
}

/*
	CSP_GENERATOR_NAIVE_BAYES::PROBABILITY_CMP()
	--------------------------------------------
	Used to sort items based on their probability of occuring.
*/
int CSP_generator_naive_bayes::probability_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	return (x->probability < y->probability) - (x->probability > y->probability);
}

/*
	CSP_GENERATOR_NAIVE_BAYES::CALCULATE_PROBABILITY()
	--------------------------------------------------
	Calculates the Naive Bayes probability of being able to rate a movie, given that we have been able to rate so far. Only have to acccount for the new non/ratable ones.
*/
double CSP_generator_naive_bayes::calculate_probability(uint64_t movie, uint64_t non_ratable, uint64_t ratable)
{
	uint64_t this_count, that_count, co_raters;
	uint64_t *this_one, *that_one;
	uint64_t presented_movie, i, j;
	double probability;
	double p_that_given_this, p_this, p_that, p_not_that, p_not_that_given_this;
	
	p_not_that = 1.0;
	p_not_that_given_this = 1.0;
	presented_movie = 1;
	non_ratable = non_ratable;
	
	this_one = dataset->ratings_for_movie(movie, &this_count);
	p_this = 1.0 * this_count / dataset->number_items;
	
	/*
		Take care of the one they could rate, this gives us something to multiply on for the ones they couldn't rate.
	*/
	that_one = dataset->ratings_for_movie(presentation_list[ratable], &that_count);
	i = MIN(presentation_list[ratable], movie);
	j = MAX(presentation_list[ratable], movie);
	co_raters = coraters[CORR(i, j)];
	
	p_that_given_this = 1.0 * co_raters / this_count;
	p_that = 1.0 * that_count / dataset->number_items;
	
	probability = p_that_given_this * p_this / p_that;
	
	
	/*
		Consider each movie that they couldn't rate.
	*/
	//for (presented_movie = non_ratable; presented_movie < ratable; presented_movie++)
	//{
	//	that_one = dataset->ratings_for_movie(presentation_list[presented_movie], &that_count);
	//	i = MIN(presentation_list[presented_movie], movie);
	//	j = MAX(presentation_list[presented_movie], movie);
	//	co_raters = coraters[CORR(i, j)];
	//	
	//	p_not_that_given_this = 1.0 * (this_count - co_raters) / (dataset->number_items - this_count);
	//	p_not_that = 1.0 * (dataset->number_items - that_count) / dataset->number_items;
	//	
	//	probability *= p_not_that_given_this * p_this / p_not_that;
	//}
	
	return probability;
}

/*
	CSP_GENERATOR_NAIVE_BAYES::GENERATE()
	-------------------------------------
*/
uint64_t *CSP_generator_naive_bayes::generate(uint64_t user, uint64_t number_presented)
{
	uint64_t i;
	
	if (number_presented == 0)
	{
		for (i = 0; i < dataset->number_items; i++)
			probabilities[i] = base_probability;
		presentation_list =  CSP_generator_entropy::generate(user, number_presented);
	}
	else
	{
		/*
			First we have to change the ids/probabilites for the remaining movies.
		*/
		for (i = number_presented; i < dataset->number_items; i++)
		{
			probabilities[presentation_list[i]] *= calculate_probability(presentation_list[i], last_presented_and_seen, number_presented - 1);
			most_probable[i].movie_id = presentation_list[i];
			most_probable[i].probability = probabilities[presentation_list[i]];
		}
		
		/*
			Sort them based on their probability of being rated.
		*/
		qsort(most_probable + number_presented, dataset->number_items - number_presented, sizeof(*most_probable), CSP_generator_naive_bayes::probability_cmp);
		
		/*
			Put the ids back into the presentation list.
		*/
		for (i = number_presented; i < dataset->number_items; i++)
			presentation_list[i] = most_probable[i].movie_id;
	}
	
	last_presented_and_seen = number_presented;
	
	return presentation_list;
}
