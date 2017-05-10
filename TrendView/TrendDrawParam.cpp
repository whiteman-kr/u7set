#include "TrendDrawParam.h"

namespace TrendLib
{

	TrendDrawParam::TrendDrawParam()
	{
	}

	QRect TrendDrawParam::rect() const
	{
		return m_rect;
	}

	void TrendDrawParam::setRect(const QRect& value)
	{
		m_rect = value;
		return;
	}

	int TrendDrawParam::dpiX() const
	{
		return m_dpiX;
	}

	int TrendDrawParam::dpiY() const
	{
		return m_dpiY;
	}

	void TrendDrawParam::setDpi(int dpiX, int dpiY)
	{
		m_dpiX = dpiX;
		m_dpiY = dpiY;
		return;
	}

	TrendView TrendDrawParam::view() const
	{
		return m_view;
	}

	void TrendDrawParam::setView(TrendView value)
	{
		m_view = value;
	}

	int TrendDrawParam::laneCount() const
	{
		return m_laneCount;
	}

	void TrendDrawParam::setLaneCount(int value)
	{
		m_laneCount = value;
	}

	QColor TrendDrawParam::backgroundColor() const
	{
		return m_backgroundColor;
	}

	void TrendDrawParam::setBackgroundColor(const QColor& value)
	{
		m_backgroundColor = value;
	}

	QDateTime TrendDrawParam::startTime() const
	{
		return QDateTime::fromMSecsSinceEpoch(m_startTimeStamp.timeStamp);
	}

	void TrendDrawParam::setStartTime(const QDateTime& value)
	{
		m_startTimeStamp.timeStamp = value.toMSecsSinceEpoch();
	}

	TimeStamp TrendDrawParam::startTimeStamp() const
	{
		return m_startTimeStamp;
	}

	void TrendDrawParam::setStartTimeStamp(const TimeStamp& value)
	{
		m_startTimeStamp = value;
	}

	quint64 TrendDrawParam::duration() const
	{
		return m_duration;
	}

	void TrendDrawParam::setDuration(qint64 value)
	{
		m_duration = value;

		if (m_duration < 2_sec)
		{
			m_duration = 2_sec;
		}

		if (m_duration > 24_hours)
		{
			m_duration = 24_hours;
		}
	}

}
