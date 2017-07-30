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
		m_timeStamp = value;
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

}
