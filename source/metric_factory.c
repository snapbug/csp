/*
	METRIC_FACTORY.C
	----------------
*/
#include <stdio.h>
#include "metric_factory.h"
#include "metric_mae.h"
#include "metric_nmae.h"
#include "metric_rmse.h"
#include "metric_nrmse.h"

/*
	STATIC
	------
	Declaration of the metrics that we can use.
*/
static struct {
	CSP_metric *metric;
	char *name;
	uint64_t id;
} metrics[] = {
	{ 0,                    "None",  CSP_metric_factory::NONE }, // Hardcoded to be in this position
	{ new CSP_metric_mae,   "MAE",   CSP_metric_factory::MAE },
	{ new CSP_metric_nmae,  "NMAE",  CSP_metric_factory::NMAE },
	{ new CSP_metric_rmse,  "RMSE",  CSP_metric_factory::RMSE },
	{ new CSP_metric_nrmse, "NRMSE", CSP_metric_factory::NRMSE }
};

static uint64_t number_of_metrics = sizeof(metrics) / sizeof(*metrics);

/*
	CSP_METRIC_FACTORY::CSP_METRIC_FACTORY()
	----------------------------------------
*/
CSP_metric_factory::CSP_metric_factory(CSP_dataset *dataset, CSP_predictor *predictor) : CSP_metric(dataset, predictor)
{
	metrics_to_use = CSP_metric_factory::MAE;
}

/*
	CSP_METRIC_FACTORY::UPDATE()
	----------------------------
*/
//void CSP_metric_factory::update(double predicted, double actual)
//{
//	uint64_t i;
//
//	for (i = 1; i < number_of_metrics; i++)
//		if (metrics_to_use & metrics[i].id)
//			metrics[i].metric->update(predicted, actual);
//}

/*
	CSP_METRIC_FACTORY::SCORE()
	---------------------------
*/
double CSP_metric_factory::score(uint64_t user)
{
	user = user;
	return 1.0;
}

/*
	CSP_METRIC_FACTORY::RESET()
	---------------------------
*/
//void CSP_metric_factory::reset(void)
//{
//	uint64_t i;
//
//	for (i = 1; i < number_of_metrics; i++)
//		if (metrics_to_use & metrics[i].id)
//			metrics[i].metric->reset();
//}

/*
	CSP_METRIC_FACTORY::SET_LIMITS()
	--------------------------------
*/
void CSP_metric_factory::set_limits(uint64_t high, uint64_t low)
{
	uint64_t i;

	for (i = 1; i < number_of_metrics; i++)
		if (metrics_to_use & metrics[i].id)
			metrics[i].metric->set_limits(high, low);
}

/*
	CSP_METRIC_FACTORY::PRINT()
	---------------------------
*/
void CSP_metric_factory::print(uint64_t user, uint64_t added)
{
	uint64_t i;

	for (i = 1; i < number_of_metrics; i++)
		if (metrics_to_use & metrics[i].id)
			printf("%s\t%lu\t%lu\t%f\n", metrics[i].name, user, added, metrics[i].metric->score());
}
