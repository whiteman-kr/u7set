#ifndef TRENDRULLER_H
#define TRENDRULLER_H
#include "../lib/TimeStamp.h"

namespace TrendLib
{
	class TrendRuller
	{
	public:
		TrendRuller();
		TrendRuller(TimeStamp& timeStamp);

		// Properties
		//
	public:
		TimeStamp timeStamp() const;
		void setTimeStamp(const TimeStamp& value);

		bool isShowRuller() const;
		void setShowRuller(bool value);

		bool showSignalValues() const;
		void setShowSignalValue(bool value);

	private:
		TimeStamp m_timeStamp;
		bool m_showRuller = true;
		bool m_showSignalValues = true;
	};
}


#endif // TRENDRULLER_H
