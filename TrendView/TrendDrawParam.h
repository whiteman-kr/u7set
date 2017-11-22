#ifndef DRAWPARAM_H
#define DRAWPARAM_H
#include "../lib/TimeStamp.h"
#include "../lib/Types.h"


namespace Proto
{
	class TrendParam;
}


namespace TrendLib
{
	enum class TrendViewMode
	{
		Separated,
		Overlapped
	};
}

Q_DECLARE_METATYPE(TrendLib::TrendViewMode)

namespace TrendLib
{
	class TrendDrawParam
	{
	public:
		TrendDrawParam();

	public:
		bool save(::Proto::TrendParam* message) const;
		bool load(const ::Proto::TrendParam& message);

	public:
		QRectF rect() const;
		void setRect(const QRectF& value);

		int dpiX() const;
		int dpiY() const;
		void setDpi(int dpiX, int dpiY);

		TrendViewMode viewMode() const;
		void setViewMode(TrendViewMode value);

		E::TimeType timeType() const;
		void setTimeType(E::TimeType value);

		int laneCount() const;
		void setLaneCount(int value);

		QColor backColor1st() const;
		void setBackColor1st(const QColor& value);

		QColor backColor2nd() const;
		void setBackColor2nd(const QColor& value);

		QDateTime startTime() const;
		void setStartTime(const QDateTime& value);

		TimeStamp startTimeStamp() const;
		void setStartTimeStamp(const TimeStamp& value);

		quint64 duration() const;
		void setLaneDuration(qint64 value);

		int hightlightRullerIndex() const;
		void setHightlightRullerIndex(int value);
		void resetHightlightRullerIndex();

		double cosmeticPenWidth() const;

		std::vector<std::pair<QString, QRectF>>& signalDescriptionRect();
		std::vector<std::pair<QString, QRectF>>& signalDescriptionRect() const;

	private:
		QRectF m_rect;
		int m_dpiX = 96;
		int m_dpiY = 96;

		TrendViewMode m_viewMode = TrendViewMode::Separated;
		E::TimeType m_timeType = E::TimeType::Local;
		int m_laneCount = 1;

		QColor m_backColor1st = {qRgb(0xE0, 0xE0, 0xE0)};
		QColor m_backColor2nd = {qRgb(0xEA, 0xEA, 0xEA)};

		TimeStamp m_startTimeStamp = TimeStamp{QDateTime::currentDateTime().toMSecsSinceEpoch() - 1_hour};
		qint64 m_duration = 1_hour;

		int m_highlightRullerIndex = -1;
		double m_cosmeticPenWidth = 0;

		mutable std::vector<std::pair<QString, QRectF>> m_signalDescriptionRect;		// Keeps signal description Rect, which is filled while draw trend, in inches.
	};
}

Q_DECLARE_METATYPE(TrendLib::TrendDrawParam)

#endif // DRAWPARAM_H
