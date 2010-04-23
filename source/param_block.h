/*
	PARAM_BLOCK.H
	-------------
*/

#ifndef PARAM_BLOCK_H_
#define PARAM_BLOCK_H_

class CSP_param_block
{
private:
	int argc;
	char **argv;

	void generation(char *which);
	void prediction(char *which);
	void metrics(char *which);
	void statistics(char *which);

public:
	CSP_param_block(int argc, char **argv);
	~CSP_param_block() {}

	void usage(void);
	void help(void);
	void parse(void);

	uint64_t generation_method; // how are we going to generate the lists to present
	uint64_t dataset; // which dataset to use
	uint64_t prediction_method; // which prediction method to use
	uint64_t metrics_to_use; // which metrics we are using
	uint64_t testing_method; // which testing method to use (proportional/fixed)
	uint64_t load_extra; // whether to load extra data
	uint64_t stats; // which stats we are interested in
	
	enum { D_NETFLIX };
	enum { A_TIME, A_PRES };
	enum { S_FIXED, S_PROP };
};

#endif /* PARAM_BLOCK_H_ */
