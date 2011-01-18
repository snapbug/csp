/*
	STATS.H
	-------
*/

#ifndef STATS_H_
#define STATS_H_

class CSP_stats
{
public:
	CSP_stats(uint64_t stats);// { this->stats = stats; }
	virtual ~CSP_stats() {}
	
	enum {
		NONE            = 0,
		GENERATION_TIME = 1,
		PREDICTION_TIME = 2,
		ERROR_PRESENTED = 4,
		ERROR_RATED     = 8,
		AUC             = 16,
		HIT_RATE        = 32,
		FINISH          = 64,
		TTEST           = 128
	};

public:
	uint64_t stats;
};

#endif /* STATS_H_ */
