/*
	PREDICTOR_KORBELL.C
	-------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "csp_types.h"
#include "predictor_korbell.h"

/*
	Save typing, increase readability, by defining these macros to calculate the effects.
*/
#define user_movie_average(user, movie) ((user_movie_average_bottom[user] && user_counts[user] && movie_counts[movie]) ? ((user_counts[user] * (user_movie_average_effect[user] / user_movie_average_bottom[user])) / (user_counts[user] + user_movie_average_alpha)) * ((movie_average[movie] / movie_counts[movie]) - (user_movie_average_average[user] / user_counts[user])) : 0)
#define user_movie_support(user, movie) ((user_movie_support_bottom[user] && user_counts[user]) ? ((user_counts[user] * (user_movie_support_effect[user] / user_movie_support_bottom[user])) / (user_counts[user] + user_movie_support_alpha)) * (sqrt((double)movie_counts[movie]) - (user_movie_support_average[user] / user_counts[user])) : 0)
#define movie_user_average(movie, user) ((movie_user_average_bottom[movie] && movie_counts[movie] && user_counts[user]) ? ((movie_counts[movie] * (movie_user_average_effect[movie] / movie_user_average_bottom[movie])) / (movie_counts[movie] + movie_user_average_alpha)) * ((user_average[user] / user_counts[user]) - (movie_user_average_average[movie] / movie_counts[movie])) : 0)
#define movie_user_support(movie, user) ((movie_user_support_bottom[movie] && movie_counts[movie]) ? ((movie_counts[movie] * (movie_user_support_effect[movie] / movie_user_support_bottom[movie])) / (movie_counts[movie] + movie_user_support_alpha)) * (sqrt((double)user_counts[user]) - (movie_user_support_average[movie] / movie_counts[movie])) : 0)

/*
	CSP_PREDICTOR_KORBELL::CSP_PREDICTOR_KORBELL()
	----------------------------------------------
*/
CSP_predictor_korbell::CSP_predictor_korbell(CSP_dataset *dataset, double alpha, uint32_t *coraters) : CSP_predictor(dataset), alpha(alpha), coraters(coraters)
{
	uint64_t i, j, min, max, movie, user, rating;
	uint64_t *item_ratings, *user_ratings;
	uint64_t item_count, user_count;
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
	
	{
	movie_effect = new double[dataset->number_items];
	movie_counts = new uint64_t[dataset->number_items];
	user_effect = new double[dataset->number_users];
	user_counts = new uint64_t[dataset->number_users];
	//movie_time_effect = new double[dataset->number_items];
	//movie_time_bottom = new double[dataset->number_items];
	//movie_time_average = new double[dataset->number_items];
	//movie_first_ratings = new uint64_t[dataset->number_items];
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
	}
	
	residual_averages = new double[dataset->number_items];
#ifdef SINGLE
	correlation_intermediates = new float[3 * dataset->number_items];
	for (i = 0; i < dataset->number_items; i++)
#else
	correlation_intermediates = new float[3 * (tri_offset(dataset->number_items - 2, dataset->number_items - 1) + 1)];
	for (i = 0; i < tri_offset(dataset->number_items - 2, dataset->number_items - 1) + 1; i++)
#endif
	{
		correlation_intermediates[3 * i] = correlation_intermediates[(3 * i) + 1] = correlation_intermediates[(3 * i) + 2] = 0;
	}
	
	/*
		Statistical modeling section
	*/
	{
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
		//movie_time_effect[movie] = movie_time_bottom[movie] = movie_time_average[movie] = 0;
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
		DON'T Calculate the Movie X Time(Movie) effect.
	*/
	/*
	fprintf(stderr, "Calculating Movie X Time(Movie) Effect.\n");
	for (movie = 0; movie < dataset->number_items; movie++)
	{
		item_ratings = dataset->ratings_for_movie(movie, &movie_counts[movie]);
		movie_first_ratings[movie] = dataset->day(item_ratings);
		
		// Find first rating day for this movie.
		for (i = 1; i < movie_counts[movie]; i++)
			movie_first_ratings[movie] = movie_first_ratings[movie] < dataset->day(item_ratings[i]) ? movie_first_ratings[movie] : dataset->day(item_ratings[i]);
		
		// Find the average square root days since first rating.
		for (i = 0; i < movie_counts[movie]; i++)
			movie_time_average[movie] += sqrt((double)dataset->day(item_ratings[i]) - movie_first_ratings[movie]);
		movie_time_average[movie] /= movie_counts[movie];
		
		// Calculate the top/bottom parts for the effect calculation.
		for (i = 0; i < movie_counts[movie]; i++)
		{
			rating = dataset->rating(item_ratings[i]);
			user = dataset->user(item_ratings[i]);
			day = dataset->day(item_ratings[i]);
			prediction = global_average + (movie_effect[movie] / (movie_counts[movie] + movie_alpha)) + (user_effect[user] / (user_counts[user] + user_alpha));
			movie_time_effect[movie] += (rating - prediction) * (sqrt((double)day - movie_first_ratings[movie]) - movie_time_average[movie]);
			movie_time_bottom[movie] += pow(sqrt((double)day - movie_first_ratings[movie]) - movie_time_average[movie], 2);
		}
	}
	*/
	
	/*
		Calculate the User X Movie(Average) effect.
	*/
	fprintf(stderr, "Calculating User X Movie(Average) Effect.\n");
	for (user = 0; user < dataset->number_users; user++)
	{
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
	for (user = 0; user < dataset->number_users; user++)
	{
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
	for (movie = 0; movie < dataset->number_items; movie++)
	{
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
	for (movie = 0; movie < dataset->number_items; movie++)
	{
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
	}
	
	/*
		Now pre-calculate the portions needed for pearson correlation.
	*/
	fprintf(stderr, "Calculating average residuals for movies.\n");
	for (movie = 0; movie < dataset->number_items; movie++)
	{
		item_ratings = dataset->ratings_for_movie(movie, &item_count);
		residual_averages[movie] = 0;
		for (i = 0; i < item_count; i++)
			residual_averages[movie] += dataset->rating(item_ratings[i]) - predict_statistics(dataset->user(item_ratings[i]), movie, dataset->day(item_ratings[i]));
	}
	
	fprintf(stderr, "Pre-calculating Pearson correlation:\n");
	double residual_i, residual_j;
#ifdef SINGLE
	movie = 2451; // Lord of the Rings: Fellowship of the Ring
#else
	for (movie = 0; movie < dataset->number_items; movie++)
#endif
	{
		//if (movie % 100 == 0) { fprintf(stderr, "\r%5lu", movie); fflush(stderr); }
		item_ratings = dataset->ratings_for_movie(movie, &item_count);
		for (i = 0; i < item_count; i++)
		{
			residual_i = dataset->rating(item_ratings[i]) - predict_statistics(dataset->user(item_ratings[i]), dataset->movie(item_ratings[i]), dataset->day(item_ratings[i]));
			
			/*
				For everyone that watched that movie.
			*/
			user_ratings = dataset->ratings_for_user(dataset->user(item_ratings[i]), &user_count);
			for (j = 0; j < user_count; j++)
			{
#ifdef SINGLE
#else
				if (movie < dataset->movie(user_ratings[j]))
#endif
				{
					/*
						Update the intermediate values.
					*/
					residual_j = dataset->rating(user_ratings[j]) - predict_statistics(dataset->user(user_ratings[j]), dataset->movie(user_ratings[j]), dataset->day(user_ratings[j]));
#ifdef SINGLE
					correlation_intermediates[(3 * dataset->movie(user_ratings[j])) + 0] += (float)((residual_i - (residual_averages[movie] / movie_counts[movie])) * (residual_j - (residual_averages[dataset->movie(user_ratings[j])] / movie_counts[dataset->movie(user_ratings[j])])));
					correlation_intermediates[(3 * dataset->movie(user_ratings[j])) + 1] += (float)(pow(residual_i - (residual_averages[movie] / movie_counts[movie]), 2));
					correlation_intermediates[(3 * dataset->movie(user_ratings[j])) + 2] += (float)(pow(residual_j - (residual_averages[dataset->movie(user_ratings[j])] / movie_counts[dataset->movie(user_ratings[j])]), 2));
#else
					correlation_intermediates[(3 * tri_offset(movie, dataset->movie(user_ratings[j]))) + 0] += (float)((residual_i - (residual_averages[movie] / movie_counts[movie])) * (residual_j - (residual_averages[dataset->movie(user_ratings[j])] / movie_counts[dataset->movie(user_ratings[j])])));
					correlation_intermediates[(3 * tri_offset(movie, dataset->movie(user_ratings[j]))) + 1] += (float)pow(residual_i - (residual_averages[movie] / movie_counts[movie]), 2);
					correlation_intermediates[(3 * tri_offset(movie, dataset->movie(user_ratings[j]))) + 2] += (float)pow(residual_j - (residual_averages[dataset->movie(user_ratings[j])] / movie_counts[dataset->movie(user_ratings[j])]), 2);
#endif
				}
			}
		}
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "Done.\n");
	
#ifdef SINGLE
	printf("FOTR: %f\n", correlation_intermediates[3 * 2451]  / (sqrt(correlation_intermediates[(3 * 2451) + 1])  * sqrt(correlation_intermediates[(3 * 2451) + 2])));
	printf("TTT: %f\n",  correlation_intermediates[3 * 11520] / (sqrt(correlation_intermediates[(3 * 11520) + 1]) * sqrt(correlation_intermediates[(3 * 11520) + 2])));
	printf("ROTK: %f\n", correlation_intermediates[3 * 14239] / (sqrt(correlation_intermediates[(3 * 14239) + 1]) * sqrt(correlation_intermediates[(3 * 14239) + 2])));
#else
	printf("FOTR - TTT: %f\n",  correlation_intermediates[(3 * tri_offset(2451, 11520)) + 0]  / (sqrt(correlation_intermediates[(3 * tri_offset(2451, 11520)) + 1])  * sqrt(correlation_intermediates[(3 * tri_offset(2451, 11520)) + 2])));
	printf("FOTR - ROTK: %f\n", correlation_intermediates[(3 * tri_offset(2451, 14239)) + 0]  / (sqrt(correlation_intermediates[(3 * tri_offset(2451, 14239)) + 1])  * sqrt(correlation_intermediates[(3 * tri_offset(2451, 14239)) + 2])));
	printf("TTT - RT0K: %f\n",   correlation_intermediates[(3 * tri_offset(11520, 14239)) + 0] / (sqrt(correlation_intermediates[(3 * tri_offset(11520, 14239)) + 1]) * sqrt(correlation_intermediates[(3 * tri_offset(11520, 14239)) + 2])));
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
	CSP_PREDICTOR_KORBELL::PREDICT_NEIGHBOUR()
	------------------------------------------
*/
double CSP_predictor_korbell::predict_neighbour(uint64_t user, uint64_t movie, uint64_t day)
{
	day = day;
	user = user;
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
