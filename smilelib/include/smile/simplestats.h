#ifndef __SMILE_SIMPLESTATS_H__
#define __SMILE_SIMPLESTATS_H__

#ifndef __SMILE_TYPES_H__
#include <smile/types.h>
#endif
#ifndef __SMILE_GC_H__
#include <smile/gc.h>
#endif
#ifndef __SMILE_STRING_H__
#include <smile/string.h>
#endif

#include <math.h>

typedef struct SimpleStatsStruct {
	double min;			// Minimum value of the set.
	double max;			// Maximum value of the set.
	UInt64 count;		// Number of items in the set.

	double _oldM, _newM, _oldS, _newS;	// Internal values for tracking mean and variance.
} *SimpleStats;

Inline SimpleStats SimpleStats_Create(void)
{
	SimpleStats simpleStats;

	simpleStats = (SimpleStats)GC_MALLOC_STRUCT(struct SimpleStatsStruct);
	if (simpleStats == NULL) Smile_Abort_OutOfMemory();

	simpleStats->min = 0.0;
	simpleStats->max = 0.0;
	simpleStats->count = 0UL;

	simpleStats->_oldM = 0.0;
	simpleStats->_newM = 0.0;
	simpleStats->_oldS = 0.0;
	simpleStats->_newS = 0.0;

	return simpleStats;
}

Inline void SimpleStats_Add(SimpleStats simpleStats, double x)
{
	if (simpleStats->count++ == 0) {
		simpleStats->_oldM = simpleStats->_newM = x;
		simpleStats->_oldS = 0.0;
		simpleStats->min = x;
		simpleStats->max = x;
	}
	else {
		simpleStats->_newM = simpleStats->_oldM + (x - simpleStats->_oldM) / simpleStats->count;
		simpleStats->_newS = simpleStats->_oldS + (x - simpleStats->_oldM) * (x - simpleStats->_newM);

		simpleStats->_oldM = simpleStats->_newM;
		simpleStats->_oldS = simpleStats->_newS;

		if (x < simpleStats->min) simpleStats->min = x;
		if (x > simpleStats->max) simpleStats->max = x;
	}
}

Inline double SimpleStats_Mean(SimpleStats simpleStats)
{
	return simpleStats->count ? simpleStats->_newM : 0.0;
}

Inline double SimpleStats_Variance(SimpleStats simpleStats)
{
	return simpleStats->count > 1 ? simpleStats->_newS / (simpleStats->count - 1) : 0.0;
}

Inline double SimpleStats_StdDev(SimpleStats simpleStats)
{
	return sqrt(SimpleStats_Variance(simpleStats));
}

Inline String SimpleStats_ToString(SimpleStats simpleStats)
{
	double avg = SimpleStats_Mean(simpleStats);
	double stddev = SimpleStats_StdDev(simpleStats);

	return String_Format("count: %lu, min: %f, max: %f, avg: %f, stddev: %f",
		simpleStats->count, simpleStats->min, simpleStats->max, avg, stddev);
}

#endif
