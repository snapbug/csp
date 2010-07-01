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
#ifdef TIME_EFFECTS
	#define user_time_user(user, day) ((user_time_user_bottom[user] && user_counts[user]) ? (((user_counts[user] * (user_time_user_effect[user] / user_time_user_bottom[user])) / (user_counts[user] + user_time_user_alpha)) * (sqrt((double)(day - user_first_ratings[user])) - (user_time_user_average[user] / user_counts[user]))) : 0)
	#define user_time_movie(user, movie, day) ((user_time_movie_bottom[user] && user_counts[user]) ? (((user_counts[user] * (user_time_movie_effect[user] / user_time_movie_bottom[user])) / (user_counts[user] + user_time_movie_alpha)) * (sqrt((double)(day - movie_first_ratings[movie])) - (user_time_movie_average[user] / user_counts[user]))) : 0)
	#define movie_time_movie(movie, day) ((movie_time_movie_bottom[movie] && movie_counts[movie]) ? (((movie_counts[movie] * (movie_time_movie_effect[movie] / movie_time_movie_bottom[movie])) / (movie_counts[movie] + movie_time_movie_alpha)) * (sqrt((double)(day - movie_first_ratings[movie])) - (movie_time_movie_average[movie] / movie_counts[movie]))) : 0)
	#define movie_time_user(movie, user, day) ((movie_time_user_bottom[movie] && movie_counts[movie]) ? (((movie_counts[movie] * (movie_time_user_effect[movie] / movie_time_user_bottom[movie])) / (movie_counts[movie] + movie_time_user_alpha)) * (sqrt((double)(day - user_first_ratings[user])) - (movie_time_user_average[movie] / movie_counts[movie]))) : 0)
#else
	#define user_time_user(user, day) (0)
	#define user_time_movie(user, movie, day) (0)
	#define movie_time_movie(movie, day) (0)
	#define movie_time_user(movie, user, day) (0)
#endif
#define user_movie_average(user, movie) ((user_movie_average_bottom[user] && user_counts[user] && movie_counts[movie]) ? ((user_counts[user] * (user_movie_average_effect[user] / user_movie_average_bottom[user])) / (user_counts[user] + user_movie_average_alpha)) * ((movie_average[movie] / movie_counts[movie]) - (user_movie_average_average[user] / user_counts[user])) : 0)
#define user_movie_support(user, movie) ((user_movie_support_bottom[user] && user_counts[user]) ? ((user_counts[user] * (user_movie_support_effect[user] / user_movie_support_bottom[user])) / (user_counts[user] + user_movie_support_alpha)) * (sqrt((double)movie_counts[movie]) - (user_movie_support_average[user] / user_counts[user])) : 0)
#define movie_user_average(movie, user) ((movie_user_average_bottom[movie] && movie_counts[movie] && user_counts[user]) ? ((movie_counts[movie] * (movie_user_average_effect[movie] / movie_user_average_bottom[movie])) / (movie_counts[movie] + movie_user_average_alpha)) * ((user_average[user] / user_counts[user]) - (movie_user_average_average[movie] / movie_counts[movie])) : 0)
#define movie_user_support(movie, user) ((movie_user_support_bottom[movie] && movie_counts[movie]) ? ((movie_counts[movie] * (movie_user_support_effect[movie] / movie_user_support_bottom[movie])) / (movie_counts[movie] + movie_user_support_alpha)) * (sqrt((double)user_counts[user]) - (movie_user_support_average[movie] / movie_counts[movie])) : 0)

#define THRESHOLD (2.5e-6)

/*
	CSP_PREDICTOR_KORBELL::CSP_PREDICTOR_KORBELL()
	----------------------------------------------
*/
CSP_predictor_korbell::CSP_predictor_korbell(CSP_dataset *dataset, uint64_t k, uint32_t *coraters) : CSP_predictor(dataset), coraters(coraters), k(k)
{
	uint64_t i, min, max, movie, user, rating, day;
	uint64_t *item_ratings, *user_ratings;
	int64_t index;
	double prediction;
	
	global_average = 3.6;
	min = max = 1;

	/*
		Alpha Values from: http://www.netflixprize.com/community/viewtopic.php?pid=5563#p5563
	*/
	movie_alpha = 25;
	user_alpha = 7;
#ifdef TIME_EFFECTS
	user_time_user_alpha = 550;
	user_time_movie_alpha = 150;
	movie_time_movie_alpha = 4000;
	movie_time_user_alpha = 500;
#endif
	user_movie_average_alpha = 90;
	user_movie_support_alpha = 90;
	movie_user_average_alpha = 50;
	movie_user_support_alpha = 50;
	beta = 500;
	
	movie_effect = new double[dataset->number_items];
	movie_counts = new uint64_t[dataset->number_items];
	user_effect = new double[dataset->number_users];
	user_counts = new uint64_t[dataset->number_users];
#ifdef TIME_EFFECTS
	user_first_ratings = new uint64_t[dataset->number_users];
	movie_first_ratings = new uint64_t[dataset->number_items];
	user_time_user_effect = new double[dataset->number_users];
	user_time_user_bottom = new double[dataset->number_users];
	user_time_user_average = new double[dataset->number_users];
	user_time_movie_effect = new double[dataset->number_users];
	user_time_movie_bottom = new double[dataset->number_users];
	user_time_movie_average = new double[dataset->number_users];
	movie_time_movie_effect = new double[dataset->number_items];
	movie_time_movie_bottom = new double[dataset->number_items];
	movie_time_movie_average = new double[dataset->number_items];
	movie_time_user_effect = new double[dataset->number_items];
	movie_time_user_bottom = new double[dataset->number_items];
	movie_time_user_average = new double[dataset->number_items];
#endif
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
	
	correlation = new int8_t[tri_offset(dataset->number_items - 2, dataset->number_items - 1) + 1];
	abar_tri = new int8_t[tri_offset(dataset->number_items - 2, dataset->number_items - 1) + 1];
	abar_dia = new float[dataset->number_items];
	bbar = abar_tri; // bbar's values are the same as abar's, but make this alias to make it look nicer
	
	/*
		Initialise everything.
	*/
	for (user = 0; user < dataset->number_users; user++)
	{
		user_effect[user] = 0;
		user_average[user] = 0;
#ifdef TIME_EFFECTS
		user_time_user_effect[user] = user_time_user_bottom[user] = user_time_user_average[user] = 0;
		user_time_movie_effect[user] = user_time_movie_bottom[user] = user_time_movie_average[user] = 0;
#endif
		user_movie_average_effect[user] = user_movie_average_bottom[user] = user_movie_average_average[user] = 0;
		user_movie_support_effect[user] = user_movie_average_bottom[user] = user_movie_support_average[user] = 0;
	}
	for (movie = 0; movie < dataset->number_items; movie++)
	{
		movie_effect[movie] = 0;
		movie_average[movie] = 0;
#ifdef TIME_EFFECTS
		movie_time_movie_effect[movie] = movie_time_movie_bottom[movie] = movie_time_movie_average[movie] = 0;
		movie_time_user_effect[movie] = movie_time_user_bottom[movie] = movie_time_user_average[movie] = 0;
#endif
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
	
#ifdef TIME_EFFECTS
	/*
		Calculate the User X Time(User) effect.
	*/
	fprintf(stderr, "Calculating User X Time(User) Effect.\n");
	#pragma omp parallel for private(user, user_ratings, i, rating, movie, day, prediction)
	for (index = 0; index < (int64_t)dataset->number_users; index++)
	{
		user = index;
		user_ratings = dataset->ratings_for_user(user, &user_counts[user]);
		
		user_first_ratings[user] = dataset->day(user_ratings);
		for (i = 1; i < user_counts[user]; i++)
			user_first_ratings[user] = MIN(user_first_ratings[user], dataset->day(user_ratings[i]));
		
		for (i = 0; i < user_counts[user]; i++)
			user_time_user_average[user] += sqrt((double)(dataset->day(user_ratings[i]) - user_first_ratings[user]));

		for (i = 0; i < user_counts[user]; i++)
		{
			rating = dataset->rating(user_ratings[i]);
			movie = dataset->movie(user_ratings[i]);
			day = dataset->day(user_ratings[i]);
			prediction = global_average + (movie_effect[movie] / (movie_counts[movie] + movie_alpha)) + (user_effect[user] / (user_counts[user] + user_alpha));
			user_time_user_effect[user] += (rating - prediction) * (sqrt((double)(day - user_first_ratings[user])) - (user_time_user_average[user] / user_counts[user]));
			user_time_user_bottom[user] += pow(sqrt((double)(day - user_first_ratings[user])) - (user_time_user_average[user] / user_counts[user]), 2);
		}
	}
	
	/*
		Calculate the User X Time(Movie) effect.
	*/
	fprintf(stderr, "Calculating User X Time(Movie) Effect.\n");
	for (index = 0; index < (int64_t)dataset->number_items; index++)
	{
		movie = index;
		item_ratings = dataset->ratings_for_movie(movie, &movie_counts[movie]);
		movie_first_ratings[movie] = dataset->day(item_ratings);
		for (i = 1; i < movie_counts[movie]; i++)
			movie_first_ratings[movie] = MIN(movie_first_ratings[movie], dataset->day(item_ratings[i]));
	}
	
	#pragma omp parallel for private(user, user_ratings, i, rating, movie, day, prediction)
	for (index = 0; index < (int64_t)dataset->number_users; index++)
	{
		user = index;
		user_ratings = dataset->ratings_for_user(user, &user_counts[user]);
		
		for (i = 0; i < user_counts[user]; i++)
			user_time_movie_average[user] += sqrt((double)(dataset->day(user_ratings[i]) - movie_first_ratings[dataset->movie(user_ratings[i])]));
		
		for (i = 0; i < user_counts[user]; i++)
		{
			rating = dataset->rating(user_ratings[i]);
			movie = dataset->movie(user_ratings[i]);
			day = dataset->day(user_ratings[i]);
			prediction = global_average + (movie_effect[movie] / (movie_counts[movie] + movie_alpha)) + (user_effect[user] / (user_counts[user] + user_alpha)) + user_time_user(user, day);
			
			user_time_movie_effect[user] += (rating - prediction) * (sqrt((double)(day - movie_first_ratings[movie])) - (user_time_movie_average[user] / user_counts[user]));
			user_time_movie_bottom[user] += pow(sqrt((double)(day - movie_first_ratings[movie])) - (user_time_movie_average[user] / user_counts[user]), 2);
		}
	}
	
	/*
		Calculate the Movie X Time(Movie) effect.
	*/
	fprintf(stderr, "Calculating Movie X Time(Movie) Effect.\n");
	#pragma omp parallel for private(user, item_ratings, i, rating, movie, day, prediction)
	for (index = 0; index < (int64_t)dataset->number_items; index++)
	{
		movie = index;
		item_ratings = dataset->ratings_for_movie(movie, &movie_counts[movie]);
		
		for (i = 0; i < movie_counts[movie]; i++)
			movie_time_movie_average[movie] += sqrt((double)(dataset->day(item_ratings[i]) - movie_first_ratings[movie]));

		for (i = 0; i < movie_counts[movie]; i++)
		{
			rating = dataset->rating(item_ratings[i]);
			user = dataset->user(item_ratings[i]);
			day = dataset->day(item_ratings[i]);
			prediction = global_average + (movie_effect[movie] / (movie_counts[movie] + movie_alpha)) + (user_effect[user] / (user_counts[user] + user_alpha)) + user_time_user(user, day) + user_time_movie(user, movie, day);
			movie_time_movie_effect[movie] += (rating - prediction) * (sqrt((double)(day - movie_first_ratings[movie])) - (movie_time_movie_average[movie] / movie_counts[movie]));
			movie_time_movie_bottom[movie] += pow(sqrt((double)(day - movie_first_ratings[movie])) - (movie_time_movie_average[movie] / movie_counts[movie]), 2);
		}
	}
	
	/*
		Calculate the Movie X Time(User) effect.
	*/
	fprintf(stderr, "Calculating Movie X Time(User) Effect.\n");
	#pragma omp parallel for private(user, item_ratings, i, rating, movie, day, prediction)
	for (index = 0; index < (int64_t)dataset->number_items; index++)
	{
		movie = index;
		item_ratings = dataset->ratings_for_movie(movie, &movie_counts[movie]);
		
		for (i = 0; i < movie_counts[movie]; i++)
			movie_time_user_average[movie] += sqrt((double)(dataset->day(item_ratings[i]) - user_first_ratings[dataset->user(item_ratings[i])]));
		
		for (i = 0; i < movie_counts[movie]; i++)
		{
			rating = dataset->rating(item_ratings[i]);
			user = dataset->user(item_ratings[i]);
			day = dataset->day(item_ratings[i]);
			prediction = global_average + (movie_effect[movie] / (movie_counts[movie] + movie_alpha)) + (user_effect[user] / (user_counts[user] + user_alpha)) + user_time_user(user, day) + user_time_user(user, day) + user_time_movie(user, movie, day) + movie_time_movie(movie, day);
			
			movie_time_user_effect[movie] += (rating - prediction) * (sqrt((double)(day - user_first_ratings[user])) - (movie_time_user_average[movie] / movie_counts[movie]));
			movie_time_user_bottom[movie] += pow(sqrt((double)(day - user_first_ratings[user])) - (movie_time_user_average[movie] / movie_counts[movie]), 2);
		}
	}
#endif
	
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
			day = dataset->day(user_ratings[i]);
			prediction += user_time_user(user, day) + user_time_movie(user, movie, day) + movie_time_movie(movie, day) + movie_time_user(movie, user, day);
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
			day = dataset->day(user_ratings[i]);
			prediction += user_time_user(user, day) + user_time_movie(user, movie, day) + movie_time_movie(movie, day) + movie_time_user(movie, user, day);
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
			day = dataset->day(item_ratings[i]);
			prediction += user_time_user(user, day) + user_time_movie(user, movie, day) + movie_time_movie(movie, day) + movie_time_user(movie, user, day);
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
			day = dataset->day(item_ratings[i]);
			prediction += user_time_user(user, day) + user_time_movie(user, movie, day) + movie_time_movie(movie, day) + movie_time_user(movie, user, day);
			movie_user_support_effect[movie] += (rating - prediction) * (sqrt((double)user_counts[user]) - (movie_user_support_average[movie] / movie_counts[movie]));
			movie_user_support_bottom[movie] += pow(sqrt((double)user_counts[user]) - (movie_user_support_average[movie] / movie_counts[movie]), 2);
		}
	}
	
	/*
		Load correlations.
	*/
	fprintf(stderr, "Loading correlations from file... "); fflush(stderr);
	index = fread(correlation, sizeof(*correlation), tri_offset(dataset->number_items - 2, dataset->number_items - 1) + 1, fopen("./data/netflix.correlations.byte", "rb"));
	fprintf(stderr, "done.\n"); fflush(stderr);
	
	/*
		Load pre-calculated A-bar.
	*/
	fprintf(stderr, "Loading abar from file... ");
	fread(abar_tri, sizeof(*abar_tri), tri_offset(dataset->number_items - 2, dataset->number_items - 1) + 1, fopen("./data/netflix.abar_tri.byte", "rb"));
	fread(abar_dia, sizeof(*abar_dia), dataset->number_items, fopen("./data/netflix.abar_dia.item.residual", "rb"));
	fprintf(stderr, "done.\n"); fflush(stderr);
	
	/*
		Calculate average entries for A-bar.
	*/
	bar_avg_tri = bar_avg_dia = 0;
	for (i = 0; i < tri_offset(dataset->number_items - 2, dataset->number_items - 1) + 1; i++)
		bar_avg_tri += abar_tri[i] / 127.0;
	bar_avg_tri /= tri_offset(dataset->number_items - 2, dataset->number_items - 1) + 1;
	for (i = 0; i < dataset->number_items; i++)
		bar_avg_dia += abar_dia[i];
	bar_avg_dia /= dataset->number_items;
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
	UNUSED(day);
	                                                                  // Avg MAE    Avg RMSE   Quiz
	return global_average                                             // 0.940119   1.067615   1.131419
		+ (movie_effect[movie] / (movie_counts[movie] + movie_alpha)) // 0.833987   0.983308   1.054314
		+ (user_effect[user] / (user_counts[user] + user_alpha))      // 0.755379   0.909070   0.982653
#ifdef TIME_EFFECTS
		+ user_time_user(user, day)                                   //     -          -      0.978032
		+ user_time_movie(user, movie, day)                           //     -          -      0.975434
		+ movie_time_movie(movie, day)                                //     -          -      0.973580
		+ movie_time_user(movie, user, day)                           //     -          -      0.972888
#endif
		+ user_movie_average(user, movie)                             // 0.749824   0.903716   0.968554
		+ user_movie_support(user, movie)                             // 0.744655   0.899066   0.964815
		+ movie_user_average(movie, user)                             // 0.742681   0.897352   0.963609
		+ movie_user_support(movie, user)                             // 0.742112   0.895805   0.962599
	;
}

/*
	CSP_PREDICTOR_KORBELL::NEIGHBOUR_COMPARE()
	------------------------------------------
*/
int CSP_predictor_korbell::neighbour_compare(const void *a, const void *b)
{
#ifdef ABS_CORR
	float x = fabs(((neighbour *)a)->correlation);
	float y = fabs(((neighbour *)b)->correlation);
#else
	float x = ((neighbour *)a)->correlation;
	float y = ((neighbour *)b)->correlation;
#endif
	
	return (x < y) - (x > y);
}

/*
	CSP_PREDICTOR_KORBELL::NON_NEGATIVE_QUADRATIC_OPT()
	---------------------------------------------------
*/
void CSP_predictor_korbell::non_negative_quadratic_opt(float *a, float *b, double *w, uint64_t size)
{
	uint64_t i, j;
	double *r = new double[size];
	double *Ar = new double[size];
	double alpha, interim, magnitude = 1, old_magnitude; // start with a resonable default magnitude? 
	uint64_t iterations = 0;
	
	/*
		Initialis weights - small positive.
	*/
	for (i = 0; i < size; i++)
		w[i] = 1e-9;
	
	do 
	{
		/*
			Calculate r <- Aw - b   FROM THE PAPER, WRONG
			Calculate r <- b - Aw   Fixed: http://www.netflixprize.com/community/viewtopic.php?id=837
		*/
		for (i = 0; i < size; i++)
		{
			r[i] = b[i];
			for (j = 0; j < size; j++)
				r[i] -= a[(i * size) + j] * w[j];
		}
		
		/*
			Non-negativity contraints, and magnitude.
		*/
		old_magnitude = magnitude;
		magnitude = 0;
		for (i = 0; i < size; i++) 
		{
			if (w[i] < 1e-10 && r[i] < 0)
				r[i] = 0;
			magnitude += r[i] * r[i];
		}
		
		/*
			Robustness tip from Yehuda, to stop divergent case.
		*/
		if ((magnitude = sqrt(magnitude)) > old_magnitude + 1)
			return;
		
		/*
			Calculate alpha <- trans(r)*r / trans(r) * Ar.
		*/
		for (i = 0; i < size; i++)
		{
			Ar[i] = 0;
			for (j = 0; j < size; j++)
				Ar[i] += a[(i * size) + j] * r[j]; // A*r
		}
		alpha = interim = 0;
		for (i = 0; i < size; i++)
			alpha += r[i] * r[i]; // trans(r) * r
		for (i = 0; i < size; i++)
			interim += r[i] * Ar[i]; // trans(r) * Ar
		alpha /= interim;
		
		/*
			Adjust step size to prevent negative values.
			Modified so that it will converge - from forums.
		*/
		for (i = 0; i < size; i++) 
			if (r[i] < 0) 
				alpha = MIN(fabs(alpha), fabs(w[i] / r[i])) * (alpha / fabs(alpha));
		if (isnan(alpha) || alpha == 0)
			alpha = 0.0001;
		
		/*
			Adjust weights.
		*/
		for (i = 0; i < size; i++)
			w[i] += alpha * r[i];
		
		for (i = 0; i < size; i++)
			if (w[i] < 1e-10)
				w[i] = 0; // robustness tip from Yehuda
		
		iterations++;
	} while (magnitude > THRESHOLD && iterations < 10000); // make sure that we'll exit at some point
	
	delete [] r;
	delete [] Ar;
}

/*
	CSP_PREDICTOR_KORBELL::PREDICT_NEIGHBOUR()
	------------------------------------------
*/
double CSP_predictor_korbell::predict_neighbour(uint64_t user, uint64_t movie, uint64_t day)
{
	UNUSED(day);
	uint64_t *user_ratings, user_count, movie_count, i, j, min, max, offset, position = 0;
	double prediction = 0;
	double *weights = new double[k];
	float *ahat = new float[k * k];
	float *bhat = new float[k];
	neighbour *neighbours = new neighbour[dataset->number_items];	
	
	user_ratings = dataset->ratings_for_user(user, &user_count);
	
	for (i = 0; i < user_count; i++)
		if (dataset->included(user_ratings[i]))
		{
			min = MIN(movie, dataset->movie(user_ratings[i]));
			max = MAX(movie, dataset->movie(user_ratings[i]));
			offset = tri_offset(min, max);
			
			neighbours[position].movie_id = dataset->movie(user_ratings[i]);
			neighbours[position].considered = TRUE;
			neighbours[position].coraters = coraters[offset];
			neighbours[position].correlation = (float)((correlation[offset] / 127.0) * (coraters[offset] / (coraters[offset] + 5.0)));
			neighbours[position].data = user_ratings[i];
			
			position++;
		}
	
	/*
		Sort to get the closest neighbours at the top.
	*/
	qsort(neighbours, position, sizeof(*neighbours), CSP_predictor_korbell::neighbour_compare);
	
	/*
		Create the A hat matrix here, from precomputed A bar values
	*/
	for (i = 0; i < MIN(k, position); i++)
		for (j = 0; j < MIN(k, position); j++)
			if (i == j)
			{
				dataset->ratings_for_movie(neighbours[i].movie_id, &movie_count);
				ahat[(i * MIN(k, position)) + j] = (float)(((movie_count * abar_dia[neighbours[i].movie_id]) + (beta * bar_avg_dia)) / (movie_count + beta));
			}
			else
			{
				min = MIN(neighbours[j].movie_id, neighbours[i].movie_id);
				max = MAX(neighbours[j].movie_id, neighbours[i].movie_id);
				offset = tri_offset(min, max);
				ahat[(i * MIN(k, position)) + j] = (float)(((coraters[offset] * (abar_tri[offset] / 127.0)) + (beta * bar_avg_tri)) / (coraters[offset] + beta));
			}
	
	/*
		Now create the b bar vector
	*/
	for (j = 0; j < MIN(k, position); j++)
	{
		min = MIN(neighbours[j].movie_id, movie);
		max = MAX(neighbours[j].movie_id, movie);
		offset = tri_offset(min, max);
		bhat[j] = (float)(((coraters[offset] * (bbar[offset] / 127.0)) + (beta * bar_avg_tri)) / (coraters[offset] + beta));
	}
	
	/*
		Now solve Aw = b for w
	*/
	non_negative_quadratic_opt(ahat, bhat, weights, MIN(k, position));
	
	/*
		Now use the weights for the prediction.
	*/
	for (i = 0; i < MIN(k, position); i++)
		prediction += weights[i] * (dataset->rating(neighbours[i].data) - predict_statistics(user, neighbours[i].movie_id, dataset->day(neighbours[i].data)));
	
	delete [] weights;
	delete [] ahat;
	delete [] bhat;
	delete [] neighbours;
	
	return prediction;
}

/*
	CSP_PREDICTOR_KORBELL::PREDICT()
	--------------------------------
*/
double CSP_predictor_korbell::predict(uint64_t user, uint64_t movie, uint64_t day)
{
	return                                   // Avg MAE   Avg RMSE  Quiz RMSE
		predict_statistics(user, movie, day) // 0.742112  0.895805  0.962599
	  + predict_neighbour(user, movie, day)  // 0.689149  0.838939  0.911633
	;
}
