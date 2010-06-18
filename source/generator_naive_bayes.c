/*
	GENERATOR_NAIVE_BAYES.C
	-----------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "generator_naive_bayes.h"

/*
	CSP_GENERATOR_NAIVE_BAYES::CSP_GENERATOR_NAIVE_BAYES()
	------------------------------------------------------
*/
CSP_generator_naive_bayes::CSP_generator_naive_bayes(CSP_dataset *dataset, uint32_t *coraters) : CSP_generator_entropy(dataset), coraters(coraters)
{
	uint64_t i;
	uint64_t item_count;

	if (!dataset->loaded_extra)
		exit(puts("Must use -e to use Naive Bayes generation method."));
	
	last_presented_and_seen = 0;
	dataset->get_ratings(&number_ratings);
	most_probable = new movie[dataset->number_items];
	probabilities = new double[dataset->number_items];

	for (i = 0; i < dataset->number_items; i++)
	{
		dataset->ratings_for_movie(i, &item_count);
		most_probable[i].movie_id = i;
		most_probable[i].probability = probabilities[i] = (1.0 * item_count / number_ratings);
	}
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
	
	                             PROD(P(Others | Movie))
	P(Movie | Others) = P(Movie) -----------------------
	                                    P(Others)
	
	                   P(Other & Movie)
	P(Other | Movie) = ----------------
	                       P(Movie)
*/
double CSP_generator_naive_bayes::calculate_probability(uint64_t movie, uint64_t non_ratable, uint64_t ratable)
{
	UNUSED(ratable);
	//uint64_t *other_ratings, other_count;
	uint64_t *movie_ratings, movie_count;
	uint64_t presented_movie, co_raters = 0;
//	uint64_t i, j;
	double probability = 1;
//	double p_other, p_not_other, p_movie_and_other, p_movie_and_not_other, p_movie_given_other, p_movie_given_not_other;
	
	presented_movie = 1;
	non_ratable = non_ratable;
	
	movie_ratings = dataset->ratings_for_movie(movie, &movie_count);
	
	/*
		Take care of the one they could rate, gives us something to multiply on for the ones they couldn't rate.
	*/
	//other_ratings = dataset->ratings_for_movie(presentation_list[ratable], &other_count);
	//i = MIN(presentation_list[ratable], movie);
	//j = MAX(presentation_list[ratable], movie);
	//co_raters = coraters[tri_offset(i, j)];
	//
//	p_other = 1.0 * other_count;
//	p_movie_and_other = 1.0 * co_raters;
//	p_movie_given_other = p_movie_and_other / p_other;

//	if (co_raters == 0)
//		probability *= (0.01 / number_ratings); // super small but non-0
//	else if (co_raters == movie_count)
//		probability *= 1;
//	else
	if (co_raters)
		probability += log(1.0 - (1.0 * co_raters / movie_count));

//	probability *= p_movie_given_other;
	
	/*
		Consider each movie that they couldn't rate.
	*/
	//for (presented_movie = non_ratable; presented_movie < ratable; presented_movie++)
	//{
	//	//other_ratings = dataset->ratings_for_movie(presentation_list[presented_movie], &other_count);
	//	i = MIN(presentation_list[presented_movie], movie);
	//	j = MAX(presentation_list[presented_movie], movie);
	//	co_raters = coraters[tri_offset(i, j)];
	//	
	//	//p_movie_and_not_other = 1.0 * (movie_count - co_raters);
	//	//p_not_other = 1.0 * (number_ratings - other_count);
	//	//p_movie_given_not_other = p_movie_and_not_other / p_not_other;
	//	//probability *= p_movie_given_not_other;
	//	probability *= (1.0 * (movie_count - co_raters) / movie_count);
	//}
	
	return probability;
}

/*
	CSP_GENERATOR_NAIVE_BAYES::GENERATE()
	-------------------------------------
*/
void CSP_generator_naive_bayes::generate(uint64_t user, uint64_t *presentation_list, uint64_t number_presented)
{
	uint64_t i, item_count;
	
	if (number_presented == 0)
	{
		for (i = 0; i < dataset->number_items; i++)
		{
			dataset->ratings_for_movie(i, &item_count);
			probabilities[i] = log(1.0 - (1.0 * item_count / number_ratings));
		}
		CSP_generator_entropy::generate(user, presentation_list, number_presented);
	}
	else
	{
		/*
			First we have to change the ids/probabilites for the remaining movies.
		*/
		for (i = number_presented; i < dataset->number_items; i++)
		{
			probabilities[presentation_list[i]] += calculate_probability(presentation_list[i], last_presented_and_seen, number_presented - 1);
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
		{
			presentation_list[i] = most_probable[i].movie_id;
//			printf("%.20f\n", most_probable[i].probability);
		}
	}
	
	last_presented_and_seen = number_presented;
}
