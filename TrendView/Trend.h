#ifndef TREND_H
#define TREND_H

#include "TrendSignal.h"
#include "TrendParam.h"
#include "TrendRuler.h"

class QPainter;

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
		void draw(QImage* image, const TrendParam& drawParam) const;
		void draw(QPainter* painter, const TrendParam& drawParam, bool needAdjustPainter) const;

		void drawLane(QPainter* painter, const QRectF& laneRect, const TrendParam& drawParam) const;

		void drawBackground(QPainter* painter,
							const QRectF& insideRect,
							const TrendParam& drawParam,
							const std::vector<TrendSignalParam>& discretes,
							const std::vector<TrendSignalParam>& analogs) const;

		void drawTimeGrid(QPainter* painter,
						  const QRectF& laneRect,
						  const QRectF& insideRect,
						  const TrendParam& drawParam) const;

		void drawSignalsDecor(QPainter* painter,
							  const QRectF& laneRect,
							  const TrendParam& drawParam,
							  const std::vector<TrendSignalParam>& discretes,
							  const std::vector<TrendSignalParam>& analogs) const;

		void drawAnalogSignalsGridSeparateMode(QPainter* painter,
											   const QRectF& laneRect,
											   const TrendParam& drawParam,
											   const TrendSignalParam& signal) const;

		void drawAnalogSignalsGridOverlappedMode(QPainter* painter,
												 const QRectF& laneRect,
												 const TrendParam& drawParam,
												 const std::vector<TrendSignalParam>& analogs) const;

		void drawSignalTrend(QPainter* painter, const TrendSignalParam& signal, const TrendParam& drawParam) const;
		void drawSignalTrendDiscrete(QPainter* painter, const TrendSignalParam& signal, const TrendParam& drawParam, const std::list<std::shared_ptr<OneHourData>>& signalData) const;
		void drawSignalTrendAnalog(QPainter* painter, const TrendSignalParam& signal, const TrendParam& drawParam, const std::list<std::shared_ptr<OneHourData>>& signalData) const;

		void drawRulers(QPainter* painter, const TrendParam& drawParam) const;
		TrendStateItem rulerSignalState(const TrendRuler& ruler, QString appSignalId, E::TimeType timeType) const;

		static void adjustPainter(QPainter* painter, int dpiX, int dpiY);

		void drawPolyline(QPainter* painter, const QVector<QPointF>& lines, const QRectF& rect) const;

	public:
		static void calcSignalRects(const QRectF& insideRect,
							 const TrendParam& drawParam,
							 std::vector<TrendSignalParam>* discretes,
							 std::vector<TrendSignalParam>* analogs);

		static QRectF calcLaneRect(int laneIndex, const TrendParam& drawParam);
		QRectF calcTrendArea(const QRectF& laneRect, const TrendParam& drawParam) const;
		static QRectF calcTrendArea(const QRectF& laneRect, const TrendParam& drawParam, size_t analogSignalCount);
		static QRectF calcScaleAreaRect(const QRectF& laneRect, const QRectF& signalRect);

		static QRect inchRectToPixelRect(const QRectF& rect, const TrendParam& drawParam);
		static QRectF pixelRectToInchRect(const QRect& rect, const TrendParam& drawParam);

		static QPoint inchPointToPixelPoint(const QPointF& point, const TrendParam& drawParam);
		static QPointF pixelPointToInchPoint(const QPoint& point, const TrendParam& drawParam);

		// Service methods
		//
	public:
		enum class MouseOn
		{
			Outside,			// Outside any posible rect
			OutsideTrendArea,	// Outside lane but in the rect
			InsideTrendArea,	// Inside lane rectangle
			OnSignalDescription,// Over Signal Description (id + caption)
			OnRuler,			// Over ruler
		};

		Trend::MouseOn mouseIsOver(QPoint mousePos, const TrendParam& drawParam, int* laneIndex, TimeStamp* outTime, int* rulerIndex, TrendSignalParam* outSignal) const;

	public:
		void validateViewLimits(const TrendParam& drawParam);

		static void drawText(QPainter* painter, const QString& str, const QRectF& rect, const TrendParam& drawParam, int flags, QRectF* boundingRect = nullptr);

		// Properties
		//
	public:
		TrendLib::TrendSignalSet& signalSet();
		const TrendLib::TrendSignalSet& signalSet() const;

		TrendLib::TrendRulerSet& rulerSet();
		const TrendLib::TrendRulerSet& rulerSet() const;

	private:
		TrendLib::TrendSignalSet m_signalSet;
		TrendLib::TrendRulerSet m_rulerSet;

		const static double discreteSignalHeight;// = 5.0 / 8.0;		// if inches
	};

}
#endif // TREND_H
