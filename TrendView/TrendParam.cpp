#include "TrendParam.h"
#include "../Proto/trends.pb.h"

namespace TrendLib
{
	TrendParam::TrendParam()
	{
	}

	TrendParam::TrendParam(ITrendDataProvider* dataProvider) :
		m_dataProvider(dataProvider)
	{
	}

	bool TrendParam::save(::Proto::TrendParam* message) const
	{
		if (message == nullptr)
		{
			return false;
		}

		message->set_view_mode(static_cast<int>(m_viewMode));
		message->set_scale_type(static_cast<int>(m_scaleType));
		message->set_time_type(static_cast<int>(m_timeType));
		message->set_lane_count(m_laneCount);

		message->set_start_time(m_startTimeStamp.timeStamp);
		message->set_duration(m_duration);

		message->set_back_color_1st(m_backColor1st.rgb());
		message->set_back_color_2nd(m_backColor2nd.rgb());

		return true;
	}

	bool TrendParam::load(const ::Proto::TrendParam& message)
	{
		if (message.IsInitialized() == false)
		{
			Q_ASSERT(message.IsInitialized());
			return false;
		}

		m_viewMode = static_cast<E::TrendViewMode>(message.view_mode());
		m_scaleType = static_cast<E::TrendScaleType>(message.scale_type());
		m_timeType = static_cast<E::TimeType>(message.time_type());
		m_laneCount = message.lane_count();

		m_startTimeStamp.timeStamp = message.start_time();
		m_duration = message.duration();

		m_backColor1st = QColor::fromRgb(message.back_color_1st());
		m_backColor2nd = QColor::fromRgb(message.back_color_2nd());

		return true;
	}

	QRectF TrendParam::rect() const
	{
		return m_rect;
	}

	void TrendParam::setRect(const QRectF& value)
	{
		m_rect = value;
		return;
	}

	double TrendParam::dpiX() const
	{
		return m_dpiX;
	}

	double TrendParam::dpiY() const
	{
		return m_dpiY;
	}

	double TrendParam::realDpiX() const
	{
		return m_dpiX * m_devicePixelRatio;
	}

	double TrendParam::realDpiY() const
	{
		return m_dpiY * m_devicePixelRatio;
	}

	double TrendParam::devicePixelRatio() const
	{
		return m_devicePixelRatio;
	}

	void TrendParam::setDpi(double dpiX, double dpiY, double devicePixelRatio)
	{
		m_dpiX = dpiX;
		m_dpiY = dpiY;
		m_devicePixelRatio = devicePixelRatio;

		m_cosmeticPenWidth = (m_dpiX >= 600) ? (1.0 / 128.0) : 0.0;

		return;
	}

	E::TrendViewMode TrendParam::viewMode() const
	{
		return m_viewMode;
	}

	void TrendParam::setViewMode(E::TrendViewMode value)
	{
		m_viewMode = value;
	}

	E::TrendScaleType TrendParam::scaleType() const
	{
		return m_scaleType;
	}

	void TrendParam::setScaleType(E::TrendScaleType value)
	{
		m_scaleType = value;
	}

	E::TimeType TrendParam::timeType() const
	{
		return m_timeType;
	}

	void TrendParam::setTimeType(E::TimeType value)
	{
		m_timeType = value;
	}

	int TrendParam::laneCount() const
	{
		return m_laneCount;
	}

	void TrendParam::setLaneCount(int value)
	{
		m_laneCount = value;
	}

	E::TrendMode TrendParam::trendMode() const
	{
		return m_trendMode;
	}

	void TrendParam::setTrendMode(E::TrendMode value)
	{
		m_trendMode = value;
		return;
	}

	TrendLib::ITrendDataProvider* TrendParam::trendDataProvider()
	{
		return m_dataProvider;
	}

	const TrendLib::ITrendDataProvider* TrendParam::trendDataProvider() const
	{
		return m_dataProvider;
	}

	void TrendParam::setTrendDataProvider(TrendLib::ITrendDataProvider* dataProvider)
	{
		m_dataProvider = dataProvider;
	}

	QColor TrendParam::backColor1st() const
	{
		return m_backColor1st;
	}

	void TrendParam::setBackColor1st(const QColor& value)
	{
		m_backColor1st = value;
	}

	QColor TrendParam::backColor2nd() const
	{
		return m_backColor2nd;
	}

	void TrendParam::setBackColor2nd(const QColor& value)
	{
		m_backColor2nd = value;
	}

	QDateTime TrendParam::startTime() const
	{
		return m_startTimeStamp.toDateTime();
	}

	void TrendParam::setStartTime(const QDateTime& value)
	{
		m_startTimeStamp.timeStamp = value.toMSecsSinceEpoch();
	}

	TimeStamp TrendParam::startTimeStamp() const
	{
		return m_startTimeStamp;
	}

	void TrendParam::setStartTimeStamp(const TimeStamp& value)
	{
		m_startTimeStamp = value;
	}

	qint64 TrendParam::duration() const
	{
		return m_duration;
	}

	void TrendParam::setLaneDuration(qint64 value)
	{
		m_duration = value;

		if (m_duration < 500_ms)
		{
			m_duration = 500_ms;
		}

		if (m_duration > 24_hours * 7)
		{
			m_duration = 24_hours * 7;
		}
	}

	int TrendParam::hightlightRulerIndex() const
	{
		return m_highlightRulerIndex;
	}

	void TrendParam::setHightlightRulerIndex(int value)
	{
		m_highlightRulerIndex = value;
	}

	void TrendParam::resetHightlightRulerIndex()
	{
		m_highlightRulerIndex = -1;
	}

	double TrendParam::cosmeticPenWidth() const
	{
		return m_cosmeticPenWidth;
	}

	std::vector<std::pair<QString, QRectF>>& TrendParam::signalDescriptionRect()
	{
		return m_signalDescriptionRect;
	}

	std::vector<std::pair<QString, QRectF>>& TrendParam::signalDescriptionRect() const
	{
		return m_signalDescriptionRect;
	}

}
