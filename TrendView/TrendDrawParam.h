#ifndef DRAWPARAM_H
#define DRAWPARAM_H
#include "../lib/TimeStamp.h"

namespace TrendLib
{
	enum class TrendView
	{
		Separated,
		Overlapped
	};
}

Q_DECLARE_METATYPE(TrendLib::TrendView)

namespace TrendLib
{
	class TrendDrawParam
	{
	public:
		TrendDrawParam();

	public:
		QRectF rect() const;
		void setRect(const QRectF& value);

		int dpiX() const;
		int dpiY() const;
		void setDpi(int dpiX, int dpiY);

		TrendView view() const;
		void setView(TrendView value);

		TimeType timeType() const;
		void setTimeType(TimeType value);

		int laneCount() const;
		void setLaneCount(int value);

		QColor backgroundColor() const;
		void setBackgroundColor(const QColor& value);

		QColor laneBackgroundColor() const;

		QDateTime startTime() const;
		void setStartTime(const QDateTime& value);

		TimeStamp startTimeStamp() const;
		void setStartTimeStamp(const TimeStamp& value);

		quint64 duration() const;
		void setDuration(qint64 value);

		int hightlightRullerIndex() const;
		void setHightlightRullerIndex(int value);
		void resetHightlightRullerIndex();

		double cosmeticPenWidth() const;

	private:
		QRectF m_rect;
		int m_dpiX = 96;
		int m_dpiY = 96;

		TrendView m_view = TrendView::Separated;
		TimeType m_timeType = TimeType::Local;
		int m_laneCount = 1;

		QColor m_backgroundColor = {qRgb(0xE0, 0xE0, 0xE0)};
		QColor m_laneBackgroundColor = {qRgb(0xEA, 0xEA, 0xEA)};

		TimeStamp m_startTimeStamp = TimeStamp{QDateTime::currentDateTime().toMSecsSinceEpoch() - 1_hour};
		qint64 m_duration = 1_hour;

		int m_highlightRullerIndex = -1;

		double m_cosmeticPenWidth = 0;
	};
}

Q_DECLARE_METATYPE(TrendLib::TrendDrawParam)

#endif // DRAWPARAM_H
