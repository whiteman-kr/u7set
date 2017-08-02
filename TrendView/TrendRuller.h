#ifndef TRENDRULLER_H
#define TRENDRULLER_H
#include <vector>
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

	class TrendRullerSet
	{
	public:
		TrendRullerSet();

	public:
		void addRuller(const TrendLib::TrendRuller& ruller);
		void deleteRuller(const TimeStamp& rullerTimeStamp);

		std::vector<TrendLib::TrendRuller> getRullers(const TimeStamp& startTime, const TimeStamp& finishTime) const;

		std::vector<TrendLib::TrendRuller>& rullers();
		const std::vector<TrendLib::TrendRuller>& rullers() const;

		TrendLib::TrendRuller& at(int index);
		const TrendLib::TrendRuller& at(int index) const;

	private:
		std::vector<TrendLib::TrendRuller> m_rullers;
	};
}


#endif // TRENDRULLER_H
