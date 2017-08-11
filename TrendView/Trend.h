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
		void draw(QPainter* painter, const TrendDrawParam& drawParam, bool needAdjustPainter) const;

		void drawLane(QPainter* painter, const QRectF& laneRect, const TrendDrawParam& drawParam) const;

		void drawBackground(QPainter* painter,
							const QRectF& insideRect,
							const TrendDrawParam& drawParam,
							const std::vector<TrendSignalParam>& discretes,
							const std::vector<TrendSignalParam>& analogs) const;

		void drawTimeGrid(QPainter* painter,
						  const QRectF& laneRect,
						  const QRectF& insideRect,
						  const TrendDrawParam& drawParam) const;

		void drawSignalsDecor(QPainter* painter,
							  const QRectF& laneRect,
							  const TrendDrawParam& drawParam,
							  const std::vector<TrendSignalParam>& discretes,
							  const std::vector<TrendSignalParam>& analogs) const;

		void drawAnalogSignalsGridSeparateMode(QPainter* painter,
											   const QRectF& laneRect,
											   const TrendDrawParam& drawParam,
											   const TrendSignalParam& signal) const;

		void drawSignalTrend(QPainter* painter, const TrendSignalParam& signal, const TrendDrawParam& drawParam) const;
		void drawSignalTrendDiscrete(QPainter* painter, const TrendSignalParam& signal, const TrendDrawParam& drawParam, const std::list<std::shared_ptr<OneHourData>>& signalData) const;
		void drawSignalTrendAnalog(QPainter* painter, const TrendSignalParam& signal, const TrendDrawParam& drawParam, const std::list<std::shared_ptr<OneHourData>>& signalData) const;

		void drawSignal(QPainter* painter, const TrendSignalParam& signal, int signalIndex, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor) const;
		void drawDiscrete(QPainter* painter, const TrendSignalParam& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor, const std::list<std::shared_ptr<OneHourData>>& signalData) const;

		void drawAnalog(QPainter* painter, const TrendSignalParam& signal, int signalIndex, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor, const std::list<std::shared_ptr<OneHourData>>& signalData) const;
		void drawAnalogTimeGrid(QPainter* painter,
								const TrendSignalParam& signal,
								int signalIndex,
								const QRectF& rect,
								const TrendDrawParam& drawParam) const;

		void drawRullers(QPainter* painter, const TrendDrawParam& drawParam) const;

		void adjustPainter(QPainter* painter, int dpiX, int dpiY) const;

		void drawPolyline(QPainter* painter, const QVector<QPointF>& lines, const QRectF& rect) const;

	public:
		void calcSignalRects(QPainter* painter,
							 const QRectF& insideRect,
							 const TrendDrawParam& drawParam,
							 std::vector<TrendSignalParam>* discretes,
							 std::vector<TrendSignalParam>* analogs) const;

		static QRectF calcLaneRect(int laneIndex, const TrendDrawParam& drawParam);
		static QRectF calcTrendArea(const QRectF& laneRect, const TrendDrawParam& drawParam);
		static QRectF calcScaleAreaRect(const QRectF& laneRect, const QRectF& signalRect);

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
			OnSignalDescription,// Over Signal Description (id +  caption)
			OnRuller,			// Over ruller
		};

		Trend::MouseOn mouseIsOver(QPoint mousePos, const TrendDrawParam& drawParam, int* laneIndex, TimeStamp* outTime, int* rullerIndex, QString* outSignalId) const;

	public:
		static double timeToScaledPixel(const TimeStamp& time, const QRectF& rect, const TimeStamp& startTime, qint64 duration);
		static double valueToScaledPixel(double value, const QRectF& rect, double lowLimit, double highLimit);

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

		const double discreteSignalHeight = 5.0 / 8.0;		// of inch
	};

}
#endif // TREND_H
