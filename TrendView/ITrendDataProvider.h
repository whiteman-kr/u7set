#ifndef ITRENDDATAPROVIDER_H
#define ITRENDDATAPROVIDER_H
#include "../CommonLib/Times.h"
#include <list>
#include <memory>

namespace TrendLib
{
	struct OneHourData;
	class TrendSignalParam;

	class ITrendDataProvider
	{
	public:
		// Get copy of trend data for specified signal.
		// trendUuid: is used in case of SchemaItemIndicator in Trend mode to identify Connection by SchemaItem::guid()
		//
		virtual bool trendData(QUuid trendUuid,
							   const TrendLib::TrendSignalParam& trendSignal,
							   QDateTime from,
							   QDateTime to,
							   E::TimeType timeType,
							   E::TrendMode mode,
							   std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const = 0;

		// Get maximum TimeStamp buy all signals
		//
		virtual TimeStamp maxTimeStamp(QUuid trendUuid, E::TimeType timeType) const = 0;
	};

}

#endif // ITRENDDATAPROVIDER_H
