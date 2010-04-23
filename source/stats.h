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
		NONE                = 0,
		GENERATION_TIME     = 1,
		GENERATION_AUC      = 2,
		PREDICTION_TIME     = 4,
		PREDICTION_ACCURACY = 8
	};

private:
	uint64_t stats;
};

#endif /* STATS_H_ */
