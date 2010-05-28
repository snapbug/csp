/*
	PREDICTOR_KORBELL.C
	-------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>
#include "csp_types.h"
#include "predictor_korbell.h"

/*
	Save typing, increase readability, by defining these macros to calculate the effects.
*/
#define user_movie_average(user, movie) ((user_movie_average_bottom[user] && user_counts[user] && movie_counts[movie]) ? ((user_counts[user] * (user_movie_average_effect[user] / user_movie_average_bottom[user])) / (user_counts[user] + user_movie_average_alpha)) * ((movie_average[movie] / movie_counts[movie]) - (user_movie_average_average[user] / user_counts[user])) : 0)
#define user_movie_support(user, movie) ((user_movie_support_bottom[user] && user_counts[user]) ? ((user_counts[user] * (user_movie_support_effect[user] / user_movie_support_bottom[user])) / (user_counts[user] + user_movie_support_alpha)) * (sqrt((double)movie_counts[movie]) - (user_movie_support_average[user] / user_counts[user])) : 0)
#define movie_user_average(movie, user) ((movie_user_average_bottom[movie] && movie_counts[movie] && user_counts[user]) ? ((movie_counts[movie] * (movie_user_average_effect[movie] / movie_user_average_bottom[movie])) / (movie_counts[movie] + movie_user_average_alpha)) * ((user_average[user] / user_counts[user]) - (movie_user_average_average[movie] / movie_counts[movie])) : 0)
#define movie_user_support(movie, user) ((movie_user_support_bottom[movie] && movie_counts[movie]) ? ((movie_counts[movie] * (movie_user_support_effect[movie] / movie_user_support_bottom[movie])) / (movie_counts[movie] + movie_user_support_alpha)) * (sqrt((double)user_counts[user]) - (movie_user_support_average[movie] / movie_counts[movie])) : 0)

#define XX 0
#define X 1
#define XY 2
#define Y 3
#define YY 4

#ifdef SINGLE
	#define corr(a) (((correlation_intermediates[(5 * a) + XY] / coraters[a]) - ((correlation_intermediates[(5 * a) + X] / coraters[a]) * (correlation_intermediates[(5 * a) + Y] / coraters[a]))) / (sqrt((correlation_intermediates[(5 * a) + XX] / coraters[a]) - pow(correlation_intermediates[(5 * a) + X] / coraters[a], 2)) * sqrt((correlation_intermediates[(5 * a) + YY] / coraters[a]) - pow(correlation_intermediates[(5 * a) + Y] / coraters[a], 2))))
#else
	#define corr(a, b) (((correlation_intermediates[(5 * tri_offset(a, b)) + XY] / coraters[tri_offset(a, b)]) - ((correlation_intermediates[(5 * tri_offset(a, b)) + X] / coraters[tri_offset(a, b)]) * (correlation_intermediates[(5 * tri_offset(a, b)) + Y] / coraters[tri_offset(a, b)]))) / (sqrt((correlation_intermediates[(5 * tri_offset(a, b)) + XX] / coraters[tri_offset(a, b)]) - pow(correlation_intermediates[(5 * tri_offset(a, b)) + X] / coraters[tri_offset(a, b)], 2)) * sqrt((correlation_intermediates[(5 * tri_offset(a, b)) + YY] / coraters[tri_offset(a, b)]) - pow(correlation_intermediates[(5 * tri_offset(a, b)) + Y] / coraters[tri_offset(a, b)], 2))))
#endif

/*
	CSP_PREDICTOR_KORBELL::CSP_PREDICTOR_KORBELL()
	----------------------------------------------
*/
CSP_predictor_korbell::CSP_predictor_korbell(CSP_dataset *dataset, uint64_t k, uint32_t *coraters) : CSP_predictor(dataset), coraters(coraters), k(k)
{
	uint64_t i, j, min, max, movie, user, rating;
	uint64_t *item_ratings, *user_ratings;
	uint64_t item_count, user_count;
	int64_t index;
	double prediction;
	
	global_average = 3.601435;
	min = max = 1;

	/*
		Alpha Values from: http://www.netflixprize.com/community/viewtopic.php?pid=5563#p5563
	*/
	movie_alpha = 25.0;
	user_alpha = 7.0;
	movie_time_alpha = 4000.0;
	user_movie_average_alpha = 90.0;
	user_movie_support_alpha = 90.0;
	movie_user_average_alpha = 50.0;
	movie_user_support_alpha = 50.0;
	
	neighbours = new neighbour[dataset->number_items];	
	movie_effect = new double[dataset->number_items];
	movie_counts = new uint64_t[dataset->number_items];
	user_effect = new double[dataset->number_users];
	user_counts = new uint64_t[dataset->number_users];
	user_movie_average_effect = new double[dataset->number_users];
	user_movie_average_bottom = new double[dataset->number_users];
	user_movie_average_average = new double[dataset->number_users];
	movie_average = new double[dataset->number_items];
	user_movie_support_effect = new double[dataset->number_users];
	user_movie_support_bottom = new double[dataset->number_users];
	user_movie_support_average = new double[dataset->number_users];
	movie_user_average_effect = new double[dataset->number_items];
	movie_user_average_bottom = new double[dataset->number_items];
	movie_user_average_average = new double[dataset->number_items];
	user_average = new double[dataset->number_users];
	movie_user_support_effect = new double[dataset->number_items];
	movie_user_support_bottom = new double[dataset->number_items];
	movie_user_support_average = new double[dataset->number_items];
	residual_averages = new double[dataset->number_items];
	
#ifdef SINGLE
	correlation_intermediates = new double[5 * dataset->number_items];
	for (i = 0; i < 5 * dataset->number_items; i++)
#else
	correlation_intermediates = new double[5 * (tri_offset(dataset->number_items - 2, dataset->number_items - 1) + 1)];
	for (i = 0; i < 5 * tri_offset(dataset->number_items - 2, dataset->number_items - 1) + 1; i++)
#endif
	{
		correlation_intermediates[i] = 0;
	}
	
	/*
		Initialise everything.
	*/
	for (user = 0; user < dataset->number_users; user++)
	{
		user_effect[user] = 0;
		user_average[user] = 0;
		user_movie_average_effect[user] = user_movie_average_bottom[user] = user_movie_average_average[user] = 0;
		user_movie_support_effect[user] = user_movie_average_bottom[user] = user_movie_support_average[user] = 0;
	}
	for (movie = 0; movie < dataset->number_items; movie++)
	{
		movie_effect[movie] = 0;
		movie_average[movie] = 0;
		movie_user_average_effect[movie] = movie_user_average_bottom[movie] = movie_user_average_average[movie] = 0;
		movie_user_support_effect[movie] = movie_user_average_bottom[movie] = movie_user_support_average[movie] = 0;
	}
	
	/*
		Calculate the movie effect, and movie averages.
	*/
	fprintf(stderr, "Calculating Movie Effect.\n");
	for (movie = 0; movie < dataset->number_items; movie++)
	{
		item_ratings = dataset->ratings_for_movie(movie, &movie_counts[movie]);
		for (i = 0; i < movie_counts[movie]; i++)
		{
			rating = dataset->rating(item_ratings[i]);
			prediction = global_average;
			movie_effect[movie] += rating - prediction;
			movie_average[movie] += (double)rating;
		}
	}
	
	/*
		Calculate the user effect.
	*/
	fprintf(stderr, "Calculating User Effect.\n");
	for (user = 0; user < dataset->number_users; user++)
	{
		user_ratings = dataset->ratings_for_user(user, &user_counts[user]);
		for (i = 0; i < user_counts[user]; i++)
		{
			movie = dataset->movie(user_ratings[i]);
			rating = dataset->rating(user_ratings[i]);
			prediction = global_average + (movie_effect[movie] / (movie_counts[movie] + movie_alpha));
			user_effect[user] += rating - prediction;
			user_average[user] += (double)rating;
		}
	}
	
	/*
		Calculate the User X Movie(Average) effect.
	*/
	fprintf(stderr, "Calculating User X Movie(Average) Effect.\n");
	#pragma omp parallel for private(i, user_ratings, rating, movie, user, prediction)
	for (index = 0; index < (int64_t)dataset->number_users; index++)
	{
		user = index;
		user_ratings = dataset->ratings_for_user(user, &user_counts[user]);
		
		for (i = 0; i < user_counts[user]; i++)
			user_movie_average_average[user] += movie_average[dataset->movie(user_ratings[i])] / movie_counts[dataset->movie(user_ratings[i])];
		
		for (i = 0; i < user_counts[user]; i++)
		{
			rating = dataset->rating(user_ratings[i]);
			movie = dataset->movie(user_ratings[i]);
			prediction = global_average + (movie_effect[movie] / (movie_counts[movie] + movie_alpha)) + (user_effect[user] / (user_counts[user] + user_alpha));
			user_movie_average_effect[user] += (rating - prediction) * ((movie_average[movie] / movie_counts[movie]) - (user_movie_average_average[user] / user_counts[user]));
			user_movie_average_bottom[user] += pow((movie_average[movie] / movie_counts[movie]) - (user_movie_average_average[user] / user_counts[user]), 2);
		}
	}
	
	/*
		Calculate the User X Movie(Support) effect.
	*/
	fprintf(stderr, "Calculating User X Movie(Support) Effect.\n");
	#pragma omp parallel for private(i, user_ratings, rating, movie, user, prediction)
	for (index = 0; index < (int64_t)dataset->number_users; index++)
	{
		user = index;
		user_ratings = dataset->ratings_for_user(user, &user_counts[user]);
		
		for (i = 0; i < user_counts[user]; i++)
			user_movie_support_average[user] += sqrt((double)movie_counts[dataset->movie(user_ratings[i])]);
		
		for (i = 0; i < user_counts[user]; i++)
		{
			rating = dataset->rating(user_ratings[i]);
			movie = dataset->movie(user_ratings[i]);
			prediction = global_average + (movie_effect[movie] / (movie_counts[movie] + movie_alpha)) + (user_effect[user] / (user_counts[user] + user_alpha)) + user_movie_average(user, movie);
			user_movie_support_effect[user] += (rating - prediction) * (sqrt((double)movie_counts[movie]) - (user_movie_support_average[user] / user_counts[user]));
			user_movie_support_bottom[user] += pow(sqrt((double)movie_counts[movie]) - (user_movie_support_average[user] / user_counts[user]), 2);
		}
	}
	
	/*
		Calculate the Movie X User(Average) effect.
	*/
	fprintf(stderr, "Calculating Movie X User(Average) Effect.\n");
	#pragma omp parallel for private(i, item_ratings, rating, movie, user, prediction)
	for (index = 0; index < (int64_t)dataset->number_items; index++)
	{
		movie = index;
		item_ratings = dataset->ratings_for_movie(movie, &movie_counts[movie]);
		
		for (i = 0; i < movie_counts[movie]; i++)
			movie_user_average_average[movie] += user_average[dataset->user(item_ratings[i])] / user_counts[dataset->user(item_ratings[i])];
		
		for (i = 0; i < movie_counts[movie]; i++)
		{
			rating = dataset->rating(item_ratings[i]);
			user = dataset->user(item_ratings[i]);
			prediction = global_average + (movie_effect[movie] / (movie_counts[movie] + movie_alpha)) + (user_effect[user] / (user_counts[user] + user_alpha)) + user_movie_average(user, movie) + user_movie_support(user, movie);
			movie_user_average_effect[movie] += (rating - prediction) * ((user_average[user] / user_counts[user]) - (movie_user_average_average[movie] / movie_counts[movie]));
			movie_user_average_bottom[movie] += pow((user_average[user] / user_counts[user]) - (movie_user_average_average[movie] / movie_counts[movie]), 2);
		}
	}
	
	/*
		Calculate the Movie X User(Support) effect.
	*/
	fprintf(stderr, "Calculating Movie X User(Support) Effect.\n");
	#pragma omp parallel for private(i, item_ratings, rating, movie, user, prediction)
	for (index = 0; index < (int64_t)dataset->number_items; index++)
	{
		movie = index;
		item_ratings = dataset->ratings_for_movie(movie, &movie_counts[movie]);
		
		for (i = 0; i < movie_counts[movie]; i++)
			movie_user_support_average[movie] += user_counts[dataset->user(item_ratings[i])];
		
		for (i = 0; i < movie_counts[movie]; i++)
		{
			rating = dataset->rating(item_ratings[i]);
			user = dataset->user(item_ratings[i]);
			prediction = global_average + (movie_effect[movie] / (movie_counts[movie] + movie_alpha)) + (user_effect[user] / (user_counts[user] + user_alpha)) + user_movie_average(user, movie) + user_movie_support(user, movie) + movie_user_average(movie, user);
			movie_user_support_effect[movie] += (rating - prediction) * (sqrt((double)user_counts[user]) - (movie_user_support_average[movie] / movie_counts[movie]));
			movie_user_support_bottom[movie] += pow(sqrt((double)user_counts[user]) - (movie_user_support_average[movie] / movie_counts[movie]), 2);
		}
	}
	
	fprintf(stderr, "Pre-calculating Pearson correlation:\n");
	double residual_i, residual_j;
	uint64_t other;
#ifdef SINGLE
	index = 2451; // Lord of the Rings: Fellowship of the Ring
#else
	#pragma omp parallel for private(i, j, movie, other, user_ratings, item_ratings, user_count, item_count, residual_i, residual_j) schedule(dynamic, 500)
	for (index = 0; index < (int64_t)dataset->number_items; index++)
#endif
	{
		movie = index;
		if (movie % 100 == 0) { fprintf(stderr, "\r%5lu", movie); fflush(stderr); }
		item_ratings = dataset->ratings_for_movie(movie, &item_count);
		/*
			For everyone that watched that movie.
		*/
		for (i = 0; i < item_count; i++)
		{
			residual_i = dataset->rating(item_ratings[i]) - predict_statistics(dataset->user(item_ratings[i]), dataset->movie(item_ratings[i]), dataset->day(item_ratings[i]));
			
			user_ratings = dataset->ratings_for_user(dataset->user(item_ratings[i]), &user_count);
			/*
				For every movie they saw.
			*/
			for (j = 0; j < user_count; j++)
			{
				other = dataset->movie(user_ratings[j]);
#ifdef SINGLE
#else
				if (movie < other)
#endif
				{
					/*
						Update the intermediate values.
					*/
					residual_j = dataset->rating(user_ratings[j]) - predict_statistics(dataset->user(user_ratings[j]), other, dataset->day(user_ratings[j]));
					
#ifdef SINGLE
correlation_intermediates[(5 * other) + XX] += pow(residual_i, 2);
correlation_intermediates[(5 * other) + X] += residual_i;
correlation_intermediates[(5 * other) + XY] += residual_i * residual_j;
correlation_intermediates[(5 * other) + Y] += residual_j;
correlation_intermediates[(5 * other) + YY] += pow(residual_j, 2);
#else
correlation_intermediates[(5 * tri_offset(movie, other)) + XX] += pow(residual_i, 2);
correlation_intermediates[(5 * tri_offset(movie, other)) + X] += residual_i;
correlation_intermediates[(5 * tri_offset(movie, other)) + XY] += residual_i * residual_j;
correlation_intermediates[(5 * tri_offset(movie, other)) + Y] += residual_j;
correlation_intermediates[(5 * tri_offset(movie, other)) + YY] += pow(residual_j, 2);
#endif
				}
			}
		}
	}
	fprintf(stderr, "\rDone.\n");
	
#ifdef SINGLE
	printf("FOTR - FOTR: %f\n", corr(2451)); // 1.000000
	printf("FOTR - TTT: %f\n", corr(11520)); // 0.777206
	printf("FOTR - ROTK: %f\n", corr(14239)); // 0.712940
#else
	printf("FOTR - TTT: %f\n", corr(2451, 11520)); // 0.777206
	printf("FOTR - ROTK: %f\n", corr(2451, 14239)); // 0.712940
	printf("TTT - RT0K: %f\n", corr(11520, 14239)); // 0.741140
#endif
}

/*
	CSP_PREDICTOR_KORBELL::ADDED_RATING()
	-------------------------------------
*/
void CSP_predictor_korbell::added_rating(uint64_t *key)
{
	uint64_t movie = dataset->movie(key), user = dataset->user(key), rating = dataset->rating(key);
	double pred = global_average;
	
	movie_counts[movie]++;
	user_counts[user]++;
	
	/*
		Movie effect.
	*/
	movie_effect[movie] += rating - pred;
	
	/*
		User effect.
	*/
	pred += movie_effect[movie] / (movie_counts[movie] + movie_alpha);
	user_effect[user] += rating - pred;
	
	/*
		User X Movie(Average).
	*/
	pred += user_effect[user] / (user_counts[user] + user_alpha);
	movie_average[movie] += (double)rating;
	user_movie_average_average[user] += movie_average[movie] / movie_counts[movie];
	user_movie_average_effect[user] += (rating - pred) * ((movie_average[movie] / movie_counts[movie]) - (user_movie_average_average[user] / user_counts[user]));
	user_movie_average_bottom[user] += pow((movie_average[movie] / movie_counts[movie]) - (user_movie_average_average[user] / user_counts[user]), 2);
	
	/*
		User X Movie(Support).
	*/
	pred += user_movie_average(user, movie);
	user_movie_support_average[user] += sqrt((double)movie_counts[movie]);
	user_movie_support_effect[user] += (rating - pred) * (sqrt((double)movie_counts[movie]) - (user_movie_support_average[user] / user_counts[user]));
	user_movie_support_bottom[user] += pow(sqrt((double)movie_counts[movie]) - (user_movie_support_average[user] / user_counts[user]), 2);
	
	/*
		Movie X User(Average)
		---------------------
		Only the users average needs to be modified, rest affects movies, which won't help/hurt.
	*/
	user_average[user] += (double)rating;
	
	/*
		Movie X User(Support)
		---------------------
		Don't have to do anything, counts already taken care of.
	*/
}

/*
	CSP_PREDICTOR_KORBELL::REMOVED_RATING()
	---------------------------------------
*/
void CSP_predictor_korbell::removed_rating(uint64_t *key)
{
	uint64_t movie = dataset->movie(key), user = dataset->user(key), rating = dataset->rating(key);
	double pred = global_average + (movie_effect[movie] / (movie_counts[movie] + movie_alpha)) + (user_effect[user] / (user_counts[user] + user_alpha)) + user_movie_average(user, movie);
	
	/*
		Movie X User(Support)
		---------------------
		Don't have to do anything - the counts are taken care of later.
		
		Movie X User(Average)
		---------------------
		Only the users average needs to be modified, rest affects movies, which won't help/hurt.
	*/
	user_average[user] -= (double)rating;
	
	/*
		User X Movie(Support) effect.
	*/
	user_movie_support_bottom[user] -= pow(sqrt((double)movie_counts[movie]) - (user_movie_support_average[user] / user_counts[user]), 2);
	user_movie_support_effect[user] -= (rating - pred) * (sqrt((double)movie_counts[movie]) - (user_movie_support_average[user] / user_counts[user]));
	user_movie_support_average[user] -= sqrt((double)movie_counts[movie]);
	
	/*
		User X Movie(Average) effect.
	*/
	pred -= user_movie_average(user, movie);
	user_movie_average_bottom[user] -= pow((movie_average[movie] / movie_counts[movie]) - (user_movie_average_average[user] / user_counts[user]), 2);
	user_movie_average_effect[user] -= (rating - pred) * ((movie_average[movie] / movie_counts[movie]) - (user_movie_average_average[user] / user_counts[user]));
	user_movie_average_average[user] -= movie_average[movie] / movie_counts[movie];
	movie_average[movie] -= (double)rating;
	
	/*
		User effect.
	*/
	pred -= user_effect[user] / (user_counts[user] + user_alpha);
	user_effect[user] -= rating - pred;
	
	/*
		Movie effect.
	*/
	pred -= movie_effect[movie] / (movie_counts[movie] + movie_alpha);
	movie_effect[movie] -= rating - pred;
	
	movie_counts[movie]--;
	user_counts[user]--;
}

/*
	CSP_PREDICTOR_KORBELL::PREDICT_STATISTICS()
	-------------------------------------------
*/
double CSP_predictor_korbell::predict_statistics(uint64_t user, uint64_t movie, uint64_t day)
{
	day = day;
	
	return global_average // 0.940119
		+ (movie_effect[movie] / (movie_counts[movie] + movie_alpha)) // 0.833987
		+ (user_effect[user] / (user_counts[user] + user_alpha)) // 0.755379
		+ user_movie_average(user, movie) // 0.749824
		+ user_movie_support(user, movie) // 0.744655
		+ movie_user_average(movie, user) // 0.742681
		+ movie_user_support(movie, user) // 0.742112
	;
}

/*
	CSP_PREDICTOR_KORBELL::NEIGHBOUR_COMPARE()
	------------------------------------------
*/
int CSP_predictor_korbell::neighbour_compare(const void *a, const void *b)
{
	neighbour *x = (neighbour *)a;
	neighbour *y = (neighbour *)b;
	
	if (x->considered && !y->considered) return -1;
	if (!x->considered && y->considered) return 1;
	if (x->correlation < y->correlation) return 1;
	if (x->correlation > y->correlation) return -1;
	return 0;
}

/*
	CSP_PREDICTOR_KORBELL::PREDICT_NEIGHBOUR()
	------------------------------------------
*/
double CSP_predictor_korbell::predict_neighbour(uint64_t user, uint64_t movie, uint64_t day)
{
//	uint64_t *user_ratings, user_count;
//	uint64_t i;
//	
//	for (i = 0; i < dataset->number_items; i++)
//	{
//		neighbours[i].movie_id = i;
//		neighbours[i].considered = FALSE;
//	}
//	
//	user_ratings = dataset->ratings_for_user(user, &user_count);
//	
//	for (i = 0; i < user_count; i++)
//		if (dataset->included(user_ratings[i]) && movie != dataset->movie(user_ratings[i]))
//		{
//			neighbours[dataset->movie(user_ratings[i])].considered = TRUE;
//			neighbours[dataset->movie(user_ratings[i])].correlation = corr(MIN(movie, dataset->movie(user_ratings[i])), MAX(movie, dataset->movie(user_ratings[i])));
//		}
//	
//	qsort(neighbours, dataset->number_items, sizeof(*neighbours), CSP_predictor_korbell::neighbour_compare);
//	
//	for (i = 0; i < dataset->number_items; i++)
//		if (neighbours[i].considered)
//			printf("%lu - %f\n", neighbours[i].movie_id, neighbours[i].correlation);
	
	user = user;
	day = day;
	movie = movie;
	return 0.0;
}

/*
	CSP_PREDICTOR_KORBELL::PREDICT()
	--------------------------------
*/
double CSP_predictor_korbell::predict(uint64_t user, uint64_t movie, uint64_t day)
{
	return predict_statistics(user, movie, day) + predict_neighbour(user, movie, day);
}
