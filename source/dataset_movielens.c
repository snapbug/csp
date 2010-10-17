/*
	DATASET_MOVIELENS.C
	-----------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dataset_movielens.h"

/*
	CSP_DATASET_MOVIELENS::CSP_DATASET_MOVIELENS()
	----------------------------------------------
*/
CSP_dataset_movielens::CSP_dataset_movielens(CSP_param_block *params) : CSP_dataset(params)
{
	char testing_filename[35];
	char training_filename[35];
	uint64_t size;
	
	minimum = 1;
	maximum = 5;
	
	number_items = 10677;
	number_users = 69878;
	number_ratings = 9029516;
	number_test_ratings = 970538;
	
	/*
		Setup the filenames correctly.
	*/
	sprintf(training_filename, "./data/ml.user.data");
	sprintf(testing_filename, "./data/ml.user.test");
	
	/*
		Now actually load the data.
	*/
	fprintf(stderr, "Loading training data... "); fflush(stderr);
	size = fread(&data, sizeof(*data), number_ratings, fopen(training_filename, "rb"));
	size = fread(&index, sizeof(*index), number_users, fopen(strcat(training_filename, ".idx"), "rb"));
	fprintf(stderr, "done.\n"); fflush(stderr);
	fprintf(stderr, "Loading testing data... "); fflush(stderr);
	size = fread(&testing_data, sizeof(*testing_data), number_test_ratings, fopen(testing_filename, "rb"));
	size = fread(&testing_index, sizeof(*testing_index), number_users, fopen(strcat(testing_filename, ".idx"), "rb"));
	fprintf(stderr, "done.\n"); fflush(stderr);
	
	if (params->load_extra)
	{
		loaded_extra = TRUE;
		fprintf(stderr, "Loading extra data... "); fflush(stderr);
		sprintf(training_filename, "./data/ml.movie.data");
		size = fread(&extra_data, sizeof(*extra_data), number_ratings, fopen(training_filename, "rb"));
		size = fread(&extra_index, sizeof(*extra_index), number_items, fopen(strcat(training_filename, ".idx"), "rb"));
		fprintf(stderr, "done.\n"); fflush(stderr);
	}
}

/*
	CSP_DATASET_MOVIELENS::RATINGS_FOR_USER()
	-----------------------------------------
*/
uint64_t *CSP_dataset_movielens::ratings_for_user(uint64_t user, uint64_t *count)
{
	*count = index[user] - (user == 0 ? 0 : index[user - 1]);
	return data + index[user] - *count;
}

/*
	CSP_DATASET_MOVIELENS::TEST_RATINGS_FOR_USER()
	----------------------------------------------
*/
uint64_t *CSP_dataset_movielens::test_ratings_for_user(uint64_t user, uint64_t *count)
{
	*count = testing_index[user] - (user == 0 ? 0 : testing_index[user - 1]);
	return testing_data + testing_index[user] - *count;
}

/*
	CSP_DATASET_MOVIELENS::RATINGS_FOR_MOVIE()
	------------------------------------------
*/
uint64_t *CSP_dataset_movielens::ratings_for_movie(uint64_t movie, uint64_t *count)
{
	*count = extra_index[movie] - (movie == 0 ? 0 : extra_index[movie - 1]);
	return extra_data + extra_index[movie] - *count;
}

/*
	CSP_DATASET_MOVIELENS::GET_RATINGS()
	------------------------------------
*/
uint64_t *CSP_dataset_movielens::get_ratings(uint64_t *count)
{
	*count = number_ratings;
	return data;
}

/*
	CSP_DATASET_MOVIELENS::GET_TEST_RATINGS()
	-----------------------------------------
*/
uint64_t *CSP_dataset_movielens::get_test_ratings(uint64_t *count)
{
	*count = number_test_ratings;
	return testing_data;
}
