#include "../lib/Types.h"

int getSamplePeriodCounter(E::RtTrendsSamplePeriod period, int lmWorkcycle_ms)
{
	if (lmWorkcycle_ms <= 0)
	{
		assert(false);
		lmWorkcycle_ms = 5;
	}

//	enum class RtTrendsSamplePeriod
//	{
//		sp_5ms,
//		sp_10ms,
//		sp_20ms,
//		sp_50ms,
//		sp_100ms,
//		sp_250ms,
//		sp_500ms,
//		sp_1s,
//		sp_5s,
//		sp_15s,
//		sp_30s,
//		sp_60s,
//	};

	const int periods_ms[] = { 5, 10, 20, 50, 100, 250, 500, 1000, 5000, 15000, 30000, 60000 };

	const int periodsCount = sizeof(periods_ms) / sizeof(const int);

	int periodIndex = static_cast<int>(period);

	if (periodIndex < 0)
	{
		assert(false);
		periodIndex = 0;
	}
	else
	{
		if (periodIndex >= periodsCount)
		{
			assert(false);
			periodIndex = periodsCount - 1;
		}
	}

	int periodCounter = periods_ms[periodIndex] / lmWorkcycle_ms;

	if (periodCounter == 0)		// may be if lmWorkcycle_ms too large
	{
		assert(false);
		periodCounter = 1;
	}

	return periodCounter;
}

