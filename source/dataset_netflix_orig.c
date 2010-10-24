/*
	DATASET_NETFLIX_ORIG.C
	----------------------
*/

#include <stdio.h>
#include <string.h>
#include "dataset_netflix_orig.h"

/*
	CSP_DATASET_NETFLIX_ORIG::CSP_DATASET_NETFLIX_ORIG()
	----------------------------------------------------
*/
CSP_dataset_netflix_orig::CSP_dataset_netflix_orig(CSP_param_block *params) : CSP_dataset(params)
{
	char testing_filename[50];
	char training_filename[50];
	uint64_t size;
	
	minimum = 1;
	maximum = 5;
	
	number_items = 17770;
	number_users = 480189;
	number_ratings = 100480507;
	number_test_ratings = 2817131;
	
	/*
		Setup the filenames correctly.
	*/
	sprintf(testing_filename, "./data/orig.judging.user");
	sprintf(training_filename, "./data/orig.netflix.users");
	
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
		sprintf(training_filename, "./data/orig.netflix.movie");
		size = fread(&extra_data, sizeof(*extra_data), number_ratings, fopen(training_filename, "rb"));
		size = fread(&extra_index, sizeof(*extra_index), number_items, fopen(strcat(training_filename, ".idx"), "rb"));
		fprintf(stderr, "done.\n"); fflush(stderr);
	}
}

/*
	CSP_DATASET_NETFLIX_ORIG::RATINGS_FOR_USER()
	--------------------------------------------
*/
uint64_t *CSP_dataset_netflix_orig::ratings_for_user(uint64_t user, uint64_t *count)
{
	*count = index[user] - (user == 0 ? 0 : index[user - 1]);
	return data + index[user] - *count;
}

/*
	CSP_DATASET_NETFLIX_ORIG::TEST_RATINGS_FOR_USER()
	-------------------------------------------------
*/
uint64_t *CSP_dataset_netflix_orig::test_ratings_for_user(uint64_t user, uint64_t *count)
{
	*count = testing_index[user] - (user == 0 ? 0 : testing_index[user - 1]);
	return testing_data + testing_index[user] - *count;
}

/*
	CSP_DATASET_NETFLIX_ORIG::RATINGS_FOR_MOVIE()
	---------------------------------------------
*/
uint64_t *CSP_dataset_netflix_orig::ratings_for_movie(uint64_t movie, uint64_t *count)
{
	*count = extra_index[movie] - (movie == 0 ? 0 : extra_index[movie - 1]);
	return extra_data + extra_index[movie] - *count;
}

/*
	CSP_DATASET_NETFLIX_ORIG::GET_RATINGS()
	---------------------------------------
*/
uint64_t *CSP_dataset_netflix_orig::get_ratings(uint64_t *count)
{
	*count = number_ratings;
	return data;
}

/*
	CSP_DATASET_NETFLIX_ORIG::GET_TEST_RATINGS()
	--------------------------------------------
*/
uint64_t *CSP_dataset_netflix_orig::get_test_ratings(uint64_t *count)
{
	*count = number_test_ratings;
	return testing_data;
}
