#include "TrendDrawParam.h"
#include "../Proto/trends.pb.h"

namespace TrendLib
{

	TrendDrawParam::TrendDrawParam()
	{
	}

	bool TrendDrawParam::save(::Proto::TrendParam* message) const
	{
		if (message == nullptr)
		{
			return false;
		}

		message->set_view_mode(static_cast<int>(m_viewMode));
		message->set_time_type(static_cast<int>(m_timeType));
		message->set_lane_count(m_laneCount);

		message->set_start_time(m_startTimeStamp.timeStamp);
		message->set_duration(m_duration);

		message->set_back_color_1st(m_backColor1st.rgb());
		message->set_back_color_2nd(m_backColor2nd.rgb());

		return true;
	}

	bool TrendDrawParam::load(const ::Proto::TrendParam& message)
	{
		if (message.IsInitialized() == false)
		{
			assert(message.IsInitialized());
			return false;
		}

		m_viewMode = static_cast<TrendViewMode>(message.view_mode());
		m_timeType = static_cast<E::TimeType>(message.time_type());
		m_laneCount = message.lane_count();

		m_startTimeStamp.timeStamp = message.start_time();
		m_duration = message.duration();

		m_backColor1st = QColor::fromRgb(message.back_color_1st());
		m_backColor2nd = QColor::fromRgb(message.back_color_2nd());

		return true;
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

	E::TimeType TrendDrawParam::timeType() const
	{
		return m_timeType;
	}

	void TrendDrawParam::setTimeType(E::TimeType value)
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

	void TrendDrawParam::setLaneDuration(qint64 value)
	{
		m_duration = value;

		if (m_duration < 500_ms)
		{
			m_duration = 500_ms;
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
