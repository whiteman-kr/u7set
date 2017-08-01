#include "TrendRuller.h"

namespace TrendLib
{

	TrendRuller::TrendRuller()
	{
	}

	TrendRuller::TrendRuller(TimeStamp& timeStamp) :
		m_timeStamp(timeStamp)
	{
	}

	TimeStamp TrendRuller::timeStamp() const
	{
		return m_timeStamp;
	}

	void TrendRuller::setTimeStamp(const TimeStamp& value)
	{
		m_timeStamp = (value.timeStamp / 5) * 5;
	}

	bool TrendRuller::isShowRuller() const
	{
		return m_showRuller;
	}

	void TrendRuller::setShowRuller(bool value)
	{
		m_showRuller = value;
	}

	bool TrendRuller::showSignalValues() const
	{
		return m_showSignalValues;
	}

	void TrendRuller::setShowSignalValue(bool value)
	{
		m_showSignalValues = value;
	}

	//
	// TrendRullerSet
	//
	TrendRullerSet::TrendRullerSet()
	{
		m_rullers.reserve(16);
	}

	void TrendRullerSet::addRuller(const TrendLib::TrendRuller& ruller)
	{
		TrendLib::TrendRuller r = ruller;
		r.setTimeStamp(ruller.timeStamp().timeStamp - ruller.timeStamp().timeStamp % 5);

		m_rullers.push_back(r);
	}

	void TrendRullerSet::deleteRuller(const TimeStamp& rullerTimeStamp)
	{
		auto it = std::remove_if(m_rullers.begin(), m_rullers.end(),
								[&rullerTimeStamp](TrendRuller& ruller)
								{
									return ruller.timeStamp() == rullerTimeStamp;
								});
		m_rullers.erase(it);
		return;
	}

	std::vector<TrendLib::TrendRuller> TrendRullerSet::getRullers(const TimeStamp& startTime, const TimeStamp& finishTime) const
	{
		std::vector<TrendLib::TrendRuller> result;
		result.reserve(8);

		for (const TrendLib::TrendRuller& r : m_rullers)
		{
			if (r.timeStamp() >= startTime && r.timeStamp() <= finishTime)
			{
				result.push_back(r);
			}
		}

		return result;
	}

	std::vector<TrendLib::TrendRuller>& TrendRullerSet::rullers()
	{
		return m_rullers;
	}

	const std::vector<TrendLib::TrendRuller>& TrendRullerSet::rullers() const
	{
		return m_rullers;
	}

	TrendLib::TrendRuller& TrendRullerSet::at(int index)
	{
		return m_rullers.at(index);
	}

	const TrendLib::TrendRuller& TrendRullerSet::at(int index) const
	{
		return m_rullers.at(index);
	}


}
