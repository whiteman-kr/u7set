#ifndef TREND_H
#define TREND_H

#include "TrendSignal.h"
#include "TrendDrawParam.h"
#include "TrendRuller.h"

namespace Proto
{
	class Trend;
}

namespace TrendLib
{

	class Trend
	{
	public:
		Trend();

		// Methods
		//
	public:

		// Serialization
		//
		bool save(::Proto::Trend* message) const;
		bool load(const ::Proto::Trend& message);

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

		void drawAnalogSignalsGridOverlappedMode(QPainter* painter,
												 const QRectF& laneRect,
												 const TrendDrawParam& drawParam,
												 const std::vector<TrendSignalParam>& analogs) const;

		void drawSignalTrend(QPainter* painter, const TrendSignalParam& signal, const TrendDrawParam& drawParam) const;
		void drawSignalTrendDiscrete(QPainter* painter, const TrendSignalParam& signal, const TrendDrawParam& drawParam, const std::list<std::shared_ptr<OneHourData>>& signalData) const;
		void drawSignalTrendAnalog(QPainter* painter, const TrendSignalParam& signal, const TrendDrawParam& drawParam, const std::list<std::shared_ptr<OneHourData>>& signalData) const;

		void drawRullers(QPainter* painter, const TrendDrawParam& drawParam) const;
		TrendStateItem rullerSignalState(const TrendRuller& ruller, QString appSignalId, TimeType timeType) const;

		static void adjustPainter(QPainter* painter, int dpiX, int dpiY);

		void drawPolyline(QPainter* painter, const QVector<QPointF>& lines, const QRectF& rect) const;

	public:
		static void calcSignalRects(const QRectF& insideRect,
							 const TrendDrawParam& drawParam,
							 std::vector<TrendSignalParam>* discretes,
							 std::vector<TrendSignalParam>* analogs);

		static QRectF calcLaneRect(int laneIndex, const TrendDrawParam& drawParam);
		QRectF calcTrendArea(const QRectF& laneRect, const TrendDrawParam& drawParam) const;
		static QRectF calcTrendArea(const QRectF& laneRect, const TrendDrawParam& drawParam, size_t analogSignalCount);
		static QRectF calcScaleAreaRect(const QRectF& laneRect, const QRectF& signalRect);

		static QRect inchRectToPixelRect(const QRectF& rect, const TrendDrawParam& drawParam);
		static QRectF pixelRectToInchRect(const QRect& rect, const TrendDrawParam& drawParam);

		static QPoint inchPointToPixelPoint(const QPointF& point, const TrendDrawParam& drawParam);
		static QPointF pixelPointToInchPoint(const QPoint& point, const TrendDrawParam& drawParam);

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

		Trend::MouseOn mouseIsOver(QPoint mousePos, const TrendDrawParam& drawParam, int* laneIndex, TimeStamp* outTime, int* rullerIndex, TrendSignalParam* outSignal) const;

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

		const static double discreteSignalHeight;// = 5.0 / 8.0;		// if inches
	};

}
#endif // TREND_H
