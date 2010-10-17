/*
	PREDICTOR_ITEM_KNN.C
	--------------------
*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "predictor_item_knn.h"

/*
	CSP_PREDICTOR_ITEM_KNN::CSP_PREDICTOR_ITEM_KNN()
	------------------------------------------------
*/
CSP_predictor_item_knn::CSP_predictor_item_knn(CSP_dataset *dataset, uint64_t k) : CSP_predictor(dataset), k(k)
{
	uint64_t i, j, l, xid, yid;
	uint64_t x_rating, y_rating;
	uint64_t *movie_ratings, *user_ratings;
	uint64_t movie_count, user_count;
	
	if (!dataset->loaded_extra)
		exit(puts("Need to have loaded extra data (-e) to use Item KNN"));
	
	/*
		Setup the default values for everything.
	*/
	intermediate = new pearson_intermediate[dataset->number_items * dataset->number_items / 2];
	most_similar = new movie[dataset->number_items];
	for (i = 0; i < dataset->number_items; i++)
	{
		most_similar[i].correlation = 0;
		most_similar[i].movie_id = 0;
		most_similar[i].considered = FALSE;
	}
	for (i = 0; i < dataset->number_items * dataset->number_items / 2; i++)
	{
		intermediate[i].x = intermediate[i].y = 0;
		intermediate[i].xx = intermediate[i].yy = intermediate[i].xy = 0;
		intermediate[i].count = 0;
	}
	
	/*
		Calculate the intermediate values for all the movie correlations (memory heavy).
	*/
	for (i = 0; i < dataset->number_items; i++)
	{
		if (i % 1000 == 0)
			printf("\r%lu", i);
		/*
			For each movie.
		*/
		movie_ratings = dataset->ratings_for_movie(i, &movie_count);
		
		/*
			For each person that rated that movie.
		*/
		for (j = 0; j < movie_count; j++)
		{
			/*
				For each other rating that user gave.
			*/
			user_ratings = dataset->ratings_for_user(dataset->user(movie_ratings[j]), &user_count);
			
			for (l = 0; l < user_count; l++)
			{
				/*
					Don't care about correlations movies have with themselves.
				*/
				if (i == dataset->movie(user_ratings[l]))
					continue;
				
				xid = MIN(i, dataset->movie(user_ratings[l]));
				yid = MAX(i, dataset->movie(user_ratings[l]));
				
				x_rating = xid == i ? dataset->rating(movie_ratings[j]) : dataset->rating(user_ratings[l]);
				y_rating = yid == i ? dataset->rating(movie_ratings[j]) : dataset->rating(user_ratings[l]);
				
				/*
					Update the intermediate values.
				*/
				intermediate[tri_offset(xid, yid, dataset->number_items)].x += x_rating;
				intermediate[tri_offset(xid, yid, dataset->number_items)].y += y_rating;
				intermediate[tri_offset(xid, yid, dataset->number_items)].xy += x_rating * y_rating;
				intermediate[tri_offset(xid, yid, dataset->number_items)].xx += x_rating * x_rating;
				intermediate[tri_offset(xid, yid, dataset->number_items)].yy += y_rating * y_rating;
				intermediate[tri_offset(xid, yid, dataset->number_items)].count++;
			}
		}
	}
}

int CSP_predictor_item_knn::similarity_cmp(const void *a, const void *b)
{
	movie *x = (movie *)a;
	movie *y = (movie *)b;
	
	if (x->considered && !y->considered) return -1;
	if (!x->considered && y->considered) return 1;
	if (x->correlation < y->correlation) return 1;
	if (x->correlation > y->correlation) return -1;
	return 0;
}

int user_rated(const void *a, const void *b)
{
	uint64_t key = *(uint64_t *)a;
	uint64_t item = (*(uint64_t *)b) >> 15 & 32767;
	
	return (key > item) - (key < item);
}

/*
	CSP_PREDICTOR_ITEM_KNN::PREDICT()
	---------------------------------
*/
double CSP_predictor_item_knn::predict(uint64_t user, uint64_t movie, uint64_t day)
{
	uint64_t i, number_included = 0;
	uint64_t user_count;
	uint64_t *user_ratings, *key;
	uint64_t xid, yid;
	double prediction_top = 0, prediction_bottom = 0;
	
	user_ratings = dataset->ratings_for_user(user, &user_count);
	day = day;
	
	for (i = 0; i < user_count; i++)
		if (dataset->included(user_ratings[i]))
			number_included++;
	
	/*
		If we haven't got any ratings for this user, we can't possibly do item-neighbourhood,
		so default to an approximation to the global average.
	*/
	if (number_included == 0)
		return 3.6;
	
	/*
		We have the intermediate structures filled out already, so we can fill out the most_similar list.
	*/
	user_ratings = dataset->ratings_for_user(user, &user_count);
	for (i = 0; i < dataset->number_items; i++)
	{
		/*
			Only fill the correlation in if the person has seen it.
		*/
		key = (uint64_t *)bsearch(&i, user_ratings, user_count, sizeof(*user_ratings), user_rated);
		if (key && dataset->included(key))
		{
			xid = MIN(i, movie);
			yid = MAX(i, movie);
			
			most_similar[i].considered = TRUE;
			most_similar[i].movie_id = i;
			most_similar[i].rating = dataset->rating(key);
			
			most_similar[i].correlation = (intermediate[tri_offset(xid, yid, dataset->number_items)].xy / intermediate[tri_offset(xid, yid, dataset->number_items)].count) - ((intermediate[tri_offset(xid, yid, dataset->number_items)].x / intermediate[tri_offset(xid, yid, dataset->number_items)].count) * (intermediate[tri_offset(xid, yid, dataset->number_items)].y / intermediate[tri_offset(xid, yid, dataset->number_items)].count));
			most_similar[i].correlation /= (sqrt((intermediate[tri_offset(xid, yid, dataset->number_items)].xx / intermediate[tri_offset(xid, yid, dataset->number_items)].count) - pow(intermediate[tri_offset(xid, yid, dataset->number_items)].x / intermediate[tri_offset(xid, yid, dataset->number_items)].count, 2)) * sqrt((intermediate[tri_offset(xid, yid, dataset->number_items)].yy / intermediate[tri_offset(xid, yid, dataset->number_items)].count) - pow(intermediate[tri_offset(xid, yid, dataset->number_items)].y / intermediate[tri_offset(xid, yid, dataset->number_items)].count, 2)));
			
			/*
				Remove NaN from correlation.
			*/
			if (isnan(most_similar[i].correlation))
				most_similar[i].correlation = 0;
		}
		else
		{
			most_similar[i].considered = FALSE;
		}
	}
	
	/*
		Sort so we get the ones with the highest correlation that we are considering first.
	*/
	qsort(most_similar, dataset->number_items, sizeof(*most_similar), CSP_predictor_item_knn::similarity_cmp);
	
	/*
		Now perform the weighted sum.
	*/
	for (i = 0; i < k; i++)
	{
		prediction_top += most_similar[i].correlation * most_similar[i].rating;
		prediction_bottom += fabs(most_similar[i].correlation);
	}
	
	return prediction_top / prediction_bottom;
}
