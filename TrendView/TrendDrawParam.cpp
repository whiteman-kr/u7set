#include "TrendDrawParam.h"

namespace TrendLib
{

	TrendDrawParam::TrendDrawParam()
	{
	}

	QRectF TrendDrawParam::rect() const
	{
		return m_rect;
	}

	void TrendDrawParam::setRect(const QRectF& value)
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

		if (m_dpiX >= 600)
		{
			m_cosmeticPenWidth = 1.0 / 128.0;
		}
		else
		{
			m_cosmeticPenWidth = 0;
		}

		return;
	}

	TrendViewMode TrendDrawParam::viewMode() const
	{
		return m_viewMode;
	}

	void TrendDrawParam::setViewMode(TrendViewMode value)
	{
		m_viewMode = value;
	}

	TimeType TrendDrawParam::timeType() const
	{
		return m_timeType;
	}

	void TrendDrawParam::setTimeType(TimeType value)
	{
		m_timeType = value;
	}

	int TrendDrawParam::laneCount() const
	{
		return m_laneCount;
	}

	void TrendDrawParam::setLaneCount(int value)
	{
		m_laneCount = value;
	}

	QColor TrendDrawParam::backColor1st() const
	{
		return m_backColor1st;
	}

	void TrendDrawParam::setBackColor1st(const QColor& value)
	{
		m_backColor1st = value;
	}

	QColor TrendDrawParam::backColor2nd() const
	{
		return m_backColor2nd;
	}

	void TrendDrawParam::setBackColor2nd(const QColor& value)
	{
		m_backColor2nd = value;
	}

	QDateTime TrendDrawParam::startTime() const
	{
		return m_startTimeStamp.toDateTime();
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

	int TrendDrawParam::hightlightRullerIndex() const
	{
		return m_highlightRullerIndex;
	}

	void TrendDrawParam::setHightlightRullerIndex(int value)
	{
		m_highlightRullerIndex = value;
	}

	void TrendDrawParam::resetHightlightRullerIndex()
	{
		m_highlightRullerIndex = -1;
	}

	double TrendDrawParam::cosmeticPenWidth() const
	{
		return m_cosmeticPenWidth;
	}

	std::vector<std::pair<QString, QRectF>>& TrendDrawParam::signalDescriptionRect()
	{
		return m_signalDescriptionRect;
	}

	std::vector<std::pair<QString, QRectF>>& TrendDrawParam::signalDescriptionRect() const
	{
		return m_signalDescriptionRect;
	}

}
