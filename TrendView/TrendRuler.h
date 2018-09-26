#ifndef TRENDRULER_H
#define TRENDRULER_H
#include <vector>
#include "../lib/TimeStamp.h"

namespace Proto
{
	class TrendRuler;
	class TrendRulerSet;
}

namespace TrendLib
{
	class TrendRuler
	{
	public:
		TrendRuler();
		TrendRuler(TimeStamp& timeStamp);

	public:
		bool save(::Proto::TrendRuler* message) const;
		bool load(const ::Proto::TrendRuler& message);

		// Properties
		//
	public:
		const TimeStamp& timeStamp() const;
		void setTimeStamp(const TimeStamp& value);

		bool isShowRuler() const;
		void setShowRuler(bool value);

		bool showSignalValues() const;
		void setShowSignalValue(bool value);

	private:
		TimeStamp m_timeStamp;
		bool m_showRuler = true;
		bool m_showSignalValues = true;
	};

	class TrendRulerSet
	{
	public:
		TrendRulerSet();

	public:
		bool save(::Proto::TrendRulerSet* message) const;
		bool load(const ::Proto::TrendRulerSet& message);

	public:
		void addRuler(const TrendLib::TrendRuler& ruler);
		void deleteRuler(const TimeStamp& rulerTimeStamp);

		std::vector<TrendLib::TrendRuler> getRulers(const TimeStamp& startTime, const TimeStamp& finishTime) const;

		std::vector<TrendLib::TrendRuler>& rulers();
		const std::vector<TrendLib::TrendRuler>& rulers() const;

		TrendLib::TrendRuler& at(int index);
		const TrendLib::TrendRuler& at(int index) const;

	private:
		std::vector<TrendLib::TrendRuler> m_rulers;
	};
}


#endif // TRENDRULER_H