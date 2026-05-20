#pragma once

#include "ITrendDataProvider.h"
#include <CommonLib/Times.h>

#include <QColor>
#include <QRectF>

namespace Proto
{
	class TrendParam;
}

namespace TrendLib
{
	class TrendParam
	{
	public:
		TrendParam();
		TrendParam(const TrendParam&) = default;
		TrendParam(TrendParam&&) = default;
		TrendParam(ITrendDataProvider* dataProvider);

		TrendParam& operator=(const TrendParam&) = default;
		TrendParam& operator=(TrendParam&&) = default;

	public:
		bool save(::Proto::TrendParam* message) const;
		bool load(const ::Proto::TrendParam& message);

	public:
		[[nodiscard]] const QRectF& rectPx() const;
		[[nodiscard]] const QRectF& rectIn() const;
		void setRectPx(const QRectF& value, double dpiX, double dpiY, double devicePixelRatio);

		[[nodiscard]] double dpiX() const;
		[[nodiscard]] double dpiY() const;
		[[nodiscard]] double realDpiX() const;
		[[nodiscard]] double realDpiY() const;
		[[nodiscard]] double devicePixelRatio() const;
		void setDpi(double dpiX, double dpiY, double devicePixelRatio);

		[[nodiscard]] E::TrendViewMode viewMode() const;
		void setViewMode(E::TrendViewMode value);

		[[nodiscard]] E::TrendScaleType scaleType() const;
		void setScaleType(E::TrendScaleType value);

		[[nodiscard]] E::TimeType timeType() const;
		void setTimeType(E::TimeType value);

		[[nodiscard]] int laneCount() const;
		void setLaneCount(int value);

		[[nodiscard]] E::TrendMode trendMode() const;
		void setTrendMode(E::TrendMode value);

		TrendLib::ITrendDataProvider* trendDataProvider();
		const TrendLib::ITrendDataProvider* trendDataProvider() const;
		void setTrendDataProvider(TrendLib::ITrendDataProvider* dataProvider);

		[[nodiscard]] QColor backColor1st() const;
		void setBackColor1st(const QColor& value);

		[[nodiscard]] QColor backColor2nd() const;
		void setBackColor2nd(const QColor& value);

		[[nodiscard]] bool showSignalIds() const;
		void setShowSignalIds(bool value);

		[[nodiscard]] bool showSignalCaptions() const;
		void setShowSignalCaptions(bool value);

		[[nodiscard]] bool showSignalScales() const;
		void setShowSignalScales(bool value);

		[[nodiscard]] bool showTimeLabels() const;
		void setShowTimeLabels(bool value);

		[[nodiscard]] bool showDateLabels() const;
		void setShowDateLabels(bool value);

		[[nodiscard]] QDateTime startTime() const;
		void setStartTime(const QDateTime& value);

		[[nodiscard]] TimeStamp startTimeStamp() const;
		void setStartTimeStamp(const TimeStamp& value);

		[[nodiscard]] qint64 duration() const;
		void setLaneDuration(qint64 value);

		[[nodiscard]] int hightlightRulerIndex() const;
		void setHightlightRulerIndex(int value);
		void resetHightlightRulerIndex();

		[[nodiscard]] double cosmeticPenWidth() const;

		[[nodiscard]] QString project() const;
		void setProject(const QString& value);

		std::vector<std::pair<QString, QRectF>>& signalDescriptionRect();
		std::vector<std::pair<QString, QRectF>>& signalDescriptionRect() const;

	private:
		QRectF m_rectPx;
		QRectF m_rectIn;

		double m_dpiX = 96;
		double m_dpiY = 96;
		double m_devicePixelRatio = 1.0;

		E::TrendViewMode m_viewMode = E::TrendViewMode::Separated;
		E::TrendScaleType m_scaleType = E::TrendScaleType::Linear;
		E::TimeType m_timeType = E::TimeType::Local;
		int m_laneCount = 1;

		E::TrendMode m_trendMode = E::TrendMode::Archive;
		ITrendDataProvider* m_dataProvider = nullptr;

		QColor m_backColor1st = {qRgb(0xEA, 0xEA, 0xEA)};
		QColor m_backColor2nd = {qRgb(0xF8, 0xF8, 0xF8)};

		bool m_showSignalIds = true;
		bool m_showSignalCaptions = true;
		bool m_showSignalScales = true;
		bool m_showTimeLabels = true;
		bool m_showDateLabels = true;

		TimeStamp m_startTimeStamp = TimeStamp{QDateTime::currentDateTime().toMSecsSinceEpoch() - 1_hour};
		qint64 m_duration = 1_hour;

		int m_highlightRulerIndex = -1;
		double m_cosmeticPenWidth = 0;

		QString m_project;

		mutable std::vector<std::pair<QString, QRectF>>
			m_signalDescriptionRect; // Keeps signal description Rect, which is filled while draw trend, in inches.
	};
} // namespace TrendLib

Q_DECLARE_METATYPE(TrendLib::TrendParam)
