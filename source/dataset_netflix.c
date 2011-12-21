/*
	DATASET_NETFLIX.C
	-----------------
*/

#include <stdio.h>
#include <string.h>
#include "dataset_netflix.h"

/*
	CSP_DATASET_NETFLIX::CSP_DATASET_NETFLIX()
	------------------------------------------
*/
CSP_dataset_netflix::CSP_dataset_netflix(CSP_param_block *params) : CSP_dataset(params)
{
	char testing_filename[35];
	char training_filename[35];
	uint64_t size;
	
	minimum = 1;
	maximum = 5;
	
	number_items = 17770; // this might be higher than needed, but not lower than what's possible
	number_users = 429584; // removed the users that have < 10 ratings, index size is still large enough for original data
	number_ratings = params->testing_method == CSP_param_block::S_PROP ? 89701538 : 95589100;
	number_test_ratings = params->testing_method == CSP_param_block::S_PROP ? 10183402 : 4295840;
	
	/*
		Setup the filenames correctly.
	*/
	sprintf(testing_filename, "./data/netflix.testing.%s", "prop");//params->testing_method == CSP_param_block::S_PROP ? "prop" : "fixed");
	sprintf(training_filename, "./data/netflix.training.%s", "prop");//params->testing_method == CSP_param_block::S_PROP ? "prop" : "fixed");
	
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
		sprintf(training_filename, "./data/netflix.movie.%s", "prop");//params->testing_method == CSP_param_block::S_PROP ? "prop" : "fixed");
		size = fread(&extra_data, sizeof(*extra_data), number_ratings, fopen(training_filename, "rb"));
		size = fread(&extra_index, sizeof(*extra_index), number_items, fopen(strcat(training_filename, ".idx"), "rb"));
		fprintf(stderr, "done.\n"); fflush(stderr);
	}
}

/*
	CSP_DATASET_NETFLIX::RATINGS_FOR_USER()
	---------------------------------------
*/
uint64_t *CSP_dataset_netflix::ratings_for_user(uint64_t user, uint64_t *count)
{
	*count = index[user] - (user == 0 ? 0 : index[user - 1]);
	return data + index[user] - *count;
}

/*
	CSP_DATASET_NETFLIX::TEST_RATINGS_FOR_USER()
	--------------------------------------------
*/
uint64_t *CSP_dataset_netflix::test_ratings_for_user(uint64_t user, uint64_t *count)
{
	*count = testing_index[user] - (user == 0 ? 0 : testing_index[user - 1]);
	return testing_data + testing_index[user] - *count;
}

/*
	CSP_DATASET_NETFLIX::RATINGS_FOR_MOVIE()
	----------------------------------------
*/
uint64_t *CSP_dataset_netflix::ratings_for_movie(uint64_t movie, uint64_t *count)
{
	*count = extra_index[movie] - (movie == 0 ? 0 : extra_index[movie - 1]);
	return extra_data + extra_index[movie] - *count;
}

/*
	CSP_DATASET_NETFLIX::GET_RATINGS()
	----------------------------------
*/
uint64_t *CSP_dataset_netflix::get_ratings(uint64_t *count)
{
	*count = number_ratings;
	return data;
}

/*
	CSP_DATASET_NETFLIX::GET_TEST_RATINGS()
	---------------------------------------
*/
uint64_t *CSP_dataset_netflix::get_test_ratings(uint64_t *count)
{
	*count = number_test_ratings;
	return testing_data;
}
