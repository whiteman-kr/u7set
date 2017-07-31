#ifndef TREND_H
#define TREND_H

#include "TrendSignal.h"
#include "TrendDrawParam.h"
#include "TrendRuller.h"

namespace TrendLib
{

	class Trend
	{
	public:
		Trend();

		// Methods
		//
	public:

		// Draw methods
		//
		void draw(QImage* image, const TrendDrawParam& drawParam) const;

		void drawLane(QPainter* painter, const QRectF& rect, const TrendDrawParam& drawParam) const;
		void drawTimeGrid(QPainter* painter, const QRectF& rect, const QRectF& insideRect, const TrendDrawParam& drawParam) const;

		void drawSignal(QPainter* painter, const TrendSignalParam& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor) const;
		void drawDiscrete(QPainter* painter, const TrendSignalParam& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor, const std::list<std::shared_ptr<OneHourData>>& signalData) const;
		void drawAnalog(QPainter* painter, const TrendSignalParam& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor, const std::list<std::shared_ptr<OneHourData>>& signalData) const;

		void drawRullers(QPainter* painter, const TrendDrawParam& drawParam) const;

		void adjustPainter(QPainter* painter, int dpiX, int dpiY) const;

	public:
		static QRectF calcLaneRect(int laneIndex, const TrendDrawParam& drawParam);
		static QRectF calcTrendArea(const QRectF& laneRect, const TrendDrawParam& drawParam);

		static QRect inchRectToPixelRect(const QRectF& rect, const TrendDrawParam& drawParam);
		static QRectF pixelRectToInchRect(const QRect& rect, const TrendDrawParam& drawParam);

		// Service methods
		//
	public:
		enum class MouseOn
		{
			Outside,			// Outside any posible rect
			OutsideTrendArea,	// Outside lane but in the rect
			InsideTrendArea,	// Inside lane rectangle
			OnRuller,			// Over ruller
		};

		Trend::MouseOn mouseIsOver(QPoint mousePos, const TrendDrawParam& drawParam, int* laneIndex, TimeStamp* outTime, int* rullerIndex) const;

	public:
		static double timeToScaledPixel(const TimeStamp& time, const QRectF& rect, const TimeStamp& startTime, qint64 duration);
		static TimeStamp pixelToTime(int pos, const TrendDrawParam& drawParam);

		static void drawText(QPainter* painter, const QString& str, const QRectF& rect, const TrendDrawParam& drawParam, int flags, QRectF* boundingRect = nullptr);

		// Properties
		//
	public:
		TrendLib::TrendSignalSet& signalSet();
		const TrendLib::TrendSignalSet& signalSet() const;

		TrendLib::TrendRullerSet& rullerSet();
		const TrendLib::TrendRullerSet& rullerSet() const;

	private:
		TrendLib::TrendSignalSet m_signalSet;
		TrendLib::TrendRullerSet m_rullerSet;
	};

}
#endif // TREND_H
