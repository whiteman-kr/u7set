#pragma once

#include <TrendView/TrendParam.h>
#include <TrendView/TrendSignalSet.h>

#include "TrendRuler.h"

class QPainter;


namespace TrendLib
{
	struct Lane
	{
		size_t index;
		QRectF laneRect;
		QDateTime startTime;
	};

	class TrendImpl
	{
	public:
		// Serialization
		//
		bool save(::Proto::Trend* message) const;
		bool load(const ::Proto::Trend& message);

	public:
		// Draw methods
		//
		void draw(QImage* image, const TrendParam& drawParam, std::stop_token stoken = std::stop_token{}) const;
		void draw(QPainter* painter, const TrendParam& drawParam, bool needAdjustPainter, std::stop_token stoken = std::stop_token{}) const;

		void drawLane(QPainter* painter, const Lane& lane, const TrendParam& drawParam, std::stop_token stoken) const;

		void drawBackground(QPainter* painter,
							const QRectF& insideRect,
							const TrendParam& drawParam,
							const std::vector<TrendSignalParam>& discretes,
							const std::vector<TrendSignalParam>& analogs) const;

		void drawTimeGrid(QPainter* painter, const QRectF& laneRect, const QRectF& insideRect, const TrendParam& drawParam) const;

		void drawSignalsDecor(QPainter* painter,
							  const Lane& laneRect,
							  const TrendParam& drawParam,
							  const std::vector<TrendSignalParam>& discretes,
							  const std::vector<TrendSignalParam>& analogs) const;

		void drawSignalsDecorRealtimeValue(QPainter* painter,
										   const Lane& lane,
										   const QRectF& signalRect,
										   const TrendParam& drawParam,
										   const TrendSignalParam& signalParam) const;

		void drawAnalogSignalsGridSeparateMode(QPainter* painter,
											   const QRectF& laneRect,
											   const TrendParam& drawParam,
											   const TrendSignalParam& signal) const;

		void drawAnalogSignalsGridOverlappedMode(QPainter* painter,
												 const QRectF& laneRect,
												 const TrendParam& drawParam,
												 const std::vector<TrendSignalParam>& analogs) const;

		void drawSignalTrend(QPainter* painter, const TrendSignalParam& signal, const TrendParam& drawParam, std::stop_token stoken) const;
		void drawSignalTrendDiscrete(QPainter* painter,
									 const TrendSignalParam& signal,
									 const TrendParam& drawParam,
									 std::list<std::shared_ptr<OneHourData>>& signalData,
									 std::stop_token stoken) const;
		void drawSignalTrendAnalog(QPainter* painter,
								   const TrendSignalParam& signal,
								   const TrendParam& drawParam,
								   std::list<std::shared_ptr<OneHourData>>& signalData,
								   std::stop_token stoken) const;

		void drawRulers(QPainter* painter, TrendParam drawParam) const;
		TrendStateItem rulerSignalState(const TrendRuler& ruler, const TrendSignalParam& signal, E::TimeType timeType) const;

		static void adjustPainter(QPainter* painter, const TrendParam& trendParam);

		void drawPolyline(QPainter* painter, const std::vector<QPointF>& lines, const QRectF& rect) const;

	public:
		static void calcSignalRects(const QRectF& insideRect,
									const TrendParam& drawParam,
									std::vector<TrendSignalParam>* discretes,
									std::vector<TrendSignalParam>* analogs);

		static QRectF calcProjectNameRect(const TrendParam& drawParam);

		static QRectF calcLaneRect(size_t laneIndex, const TrendParam& drawParam);
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
			Outside,             // Outside any possible rect
			OutsideTrendArea,    // Outside lane but in the rect
			InsideTrendArea,     // Inside lane rectangle
			OnSignalDescription, // Over Signal Description (id + caption)
			OnRuler,             // Over ruler
		};

		TrendImpl::MouseOn mouseIsOver(QPoint mousePos,
									   const TrendParam& drawParam,
									   int* laneIndex,
									   TimeStamp* outTime,
									   int* rulerIndex,
									   TrendSignalParam* outSignal) const;

	public:
		static void drawText(QPainter* painter,
							 const QString& str,
							 const QRectF& rect,
							 const TrendParam& drawParam,
							 int flags,
							 QRectF* boundingRect = nullptr);

		static QSizeF calcTextSize(QPainter* painter, const QString& str, const TrendParam& drawParam);

		// Properties
		//
	public:
		[[nodiscard]] QUuid uuid() const;
		void setUuid(QUuid value);

		[[nodiscard]] TrendLib::TrendSignalSet& signalSet();
		[[nodiscard]] const TrendLib::TrendSignalSet& signalSet() const;

		[[nodiscard]] TrendLib::TrendRulerSet& rulerSet();
		[[nodiscard]] const TrendLib::TrendRulerSet& rulerSet() const;

	private:
		QUuid m_uuid{}; // id of trend, used for SchemaItemIndicator, to identfy schemaitem

		TrendLib::TrendSignalSet m_signalSet;
		TrendLib::TrendRulerSet m_rulerSet;

		const static double discreteSignalHeight;
		const static double textSizeMm;
	};
} // namespace TrendLib