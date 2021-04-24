#pragma once

#include "../lib/TimeStamp.h"
#include "../CommonLib/Types.h"
#include <QColor>
#include <QRectF>

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
	class TrendParam
	{
	public:
		TrendParam();

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

		E::TrendScaleType scaleType() const;
		void setScaleType(E::TrendScaleType value);

		E::TimeType timeType() const;
		void setTimeType(E::TimeType value);

		int laneCount() const;
		void setLaneCount(int value);

		E::TrendMode trendMode() const;
		void setTrendMode(E::TrendMode value);

		QColor backColor1st() const;
		void setBackColor1st(const QColor& value);

		QColor backColor2nd() const;
		void setBackColor2nd(const QColor& value);

		QDateTime startTime() const;
		void setStartTime(const QDateTime& value);

		TimeStamp startTimeStamp() const;
		void setStartTimeStamp(const TimeStamp& value);

		qint64 duration() const;
		void setLaneDuration(qint64 value);

		int hightlightRulerIndex() const;
		void setHightlightRulerIndex(int value);
		void resetHightlightRulerIndex();

		double cosmeticPenWidth() const;

		std::vector<std::pair<QString, QRectF>>& signalDescriptionRect();
		std::vector<std::pair<QString, QRectF>>& signalDescriptionRect() const;

	private:
		QRectF m_rect;
		int m_dpiX = 96;
		int m_dpiY = 96;

		TrendViewMode m_viewMode = TrendViewMode::Separated;
		E::TrendScaleType m_scaleType = E::TrendScaleType::Linear;
		E::TimeType m_timeType = E::TimeType::Local;
		int m_laneCount = 1;

		E::TrendMode m_trendMode = E::TrendMode::Archive;

		QColor m_backColor1st = {qRgb(0xE0, 0xE0, 0xE0)};
		QColor m_backColor2nd = {qRgb(0xEA, 0xEA, 0xEA)};

		TimeStamp m_startTimeStamp = TimeStamp{QDateTime::currentDateTime().toMSecsSinceEpoch() - 1_hour};
		qint64 m_duration = 1_hour;

		int m_highlightRulerIndex = -1;
		double m_cosmeticPenWidth = 0;

		mutable std::vector<std::pair<QString, QRectF>> m_signalDescriptionRect;		// Keeps signal description Rect, which is filled while draw trend, in inches.
	};
}

Q_DECLARE_METATYPE(TrendLib::TrendParam)

