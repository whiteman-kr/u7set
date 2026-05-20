#include "TrendImpl.h"
#include "TrendScale.h"
#include <CUtils.h>

#include <QPainter>

#include <ranges>

#define EXPERIMENTAL_TREND_DRAW_WITH_BARS
// #define DEBUG_TIME

namespace TrendLib
{
	const double TrendImpl::discreteSignalHeight = 5.0 / 10.0; // 5/10*25.4 = 12.7 mm
	const double TrendImpl::textSizeMm = 3.4;                  // 3.4 mm

	bool TrendImpl::save(::Proto::Trend* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		bool ok = true;
		ok &= m_signalSet.save(message->mutable_signal_set());
		ok &= m_rulerSet.save(message->mutable_ruler_set());

		return ok;
	}

	bool TrendImpl::load(const ::Proto::Trend& message)
	{
		if (message.IsInitialized() == false)
		{
			Q_ASSERT(message.IsInitialized());
			return false;
		}

		bool ok = true;
		ok &= m_signalSet.load(message.signal_set());
		ok &= m_rulerSet.load(message.ruler_set());

		return ok;
	}

	void TrendImpl::draw(QImage* image, const TrendParam& drawParam, std::stop_token stoken) const
	{
		if (image == nullptr)
		{
			Q_ASSERT(image);
			return;
		}

#ifdef DEBUG_TIME
		QElapsedTimer timeMeasures;
		timeMeasures.start();
#endif

		image->fill(Qt::white);

		// --
		//
		QPainter painter(image);

		draw(&painter, drawParam, true, stoken);

#ifdef DEBUG_TIME
		{
			thread_local std::deque<qint64> elapsedMedium;
			elapsedMedium.push_back(timeMeasures.elapsed());

			while (elapsedMedium.size() > 8)
			{
				elapsedMedium.pop_front();
			}

			auto average = std::accumulate(elapsedMedium.begin(), elapsedMedium.end(), 0LL);
			average /= elapsedMedium.size();

			qDebug() << "Trend draw time: " << average << " ms";
		}
#endif
		return;
	}

	void TrendImpl::draw(QPainter* painter, const TrendParam& drawParam, bool needAdjustPainter, std::stop_token stoken) const
	{
		Q_ASSERT(painter);

		if (needAdjustPainter == true)
		{
			adjustPainter(painter, drawParam);
		}

		// Draw project description
		//
		if (drawParam.project().isEmpty() == false)
		{
			painter->setPen(Qt::black);
			painter->setBrush(Qt::black);

			QRectF textRect = calcProjectNameRect(drawParam);

			drawText(painter, drawParam.project(), textRect, drawParam, Qt::AlignLeft | Qt::AlignVCenter);

			QString dateTime = DateTimeToString::dateTimeSec(QDateTime::currentDateTime());
			drawText(painter, dateTime, textRect, drawParam, Qt::AlignRight | Qt::AlignVCenter);

#if 0
			// Debug - Draw bounding rect
			//
			{
				QPen pen(Qt::red, 0, Qt::DashLine, Qt::PenCapStyle::RoundCap);
				painter->setPen(pen);
				painter->setBrush(Qt::NoBrush);
				painter->drawRect(textRect);
			}
#endif
		}

		// --
		//
		QDateTime startTime = drawParam.startTime();

		for (size_t laneIndex = 0, laneCount = drawParam.laneCount(); laneIndex < laneCount; laneIndex++)
		{
			if (stoken.stop_requested() == true)
			{
				break;
			}

			TrendParam laneDrawParam = drawParam;
			laneDrawParam.setStartTime(startTime);

			QRectF laneRect = calcLaneRect(laneIndex, drawParam);
			Lane lane{.index = laneIndex, .laneRect = laneRect, .startTime = startTime};

			drawLane(painter, lane, laneDrawParam, stoken); // Draw whole lane

#if 0
			// Debug - Draw bounding rect by cosmetic pen
			//
			{
				QPen pen(Qt::green, 0, Qt::DashLine, Qt::PenCapStyle::RoundCap);
				painter->setPen(pen);
				painter->setBrush(Qt::NoBrush);
				painter->drawRect(laneRect);
			}
#endif

			// As laneDrawParam is a copy of drawParam, we need to copy from
			// laneDrawParam to drawParam vector signalDescriptionRect
			//
			drawParam.signalDescriptionRect().insert(drawParam.signalDescriptionRect().end(),
													 laneDrawParam.signalDescriptionRect().begin(),
													 laneDrawParam.signalDescriptionRect().end());

			startTime = startTime.addMSecs(laneDrawParam.duration());
		}

		return;
	}

	void TrendImpl::drawLane(QPainter* painter, const Lane& lane, const TrendParam& drawParam, std::stop_token stoken) const
	{
		painter->setBrush(drawParam.backColor1st());
		painter->setPen(Qt::PenStyle::NoPen);
		painter->drawRect(lane.laneRect);

		std::vector<TrendSignalParam> discretes = signalSet().discreteSignals();
		std::vector<TrendSignalParam> analogs = signalSet().analogSignals();

		// Calc InsideRect
		// +--------------------------------+
		// |   +---------------------------+|
		// |   |   insideRect (trendArea)  ||
		// |   +---------------------------+|
		// +--------------------------------+
		//
		QRectF insideRect = calcTrendArea(lane.laneRect, drawParam);

		// Calc signals rects, calculates rect will be written to discretes/analogs
		//
		calcSignalRects(insideRect, drawParam, &discretes, &analogs);

		// Draw backgrounds
		//
		drawBackground(painter, insideRect, drawParam, discretes, analogs);

		// Draw Time grid
		//
		drawTimeGrid(painter, lane.laneRect, insideRect, drawParam);

		// Draw vertical scale, signal id and caption
		//
		drawSignalsDecor(painter, lane, drawParam, discretes, analogs);

		// Draw signal trend
		//
		for (const TrendSignalParam& signal : discretes)
		{
			drawSignalTrend(painter, signal, drawParam, stoken);
		}

		for (const TrendSignalParam& signal : analogs)
		{
			drawSignalTrend(painter, signal, drawParam, stoken);
		}

		return;
	}

	void TrendImpl::drawBackground(QPainter* painter,
								   const QRectF& insideRect,
								   const TrendParam& drawParam,
								   const std::vector<TrendSignalParam>& discretes,
								   const std::vector<TrendSignalParam>& analogs) const
	{
		Q_ASSERT(painter);
		painter->setClipping(false);

		QRectF lastDiscreteRect;

		// Draw discrete signals is the same for both modes (TrendView::Separated/TrendView::Overlapped)
		//
		QColor signalBackColor = drawParam.backColor1st();

		for (const TrendSignalParam& ts : discretes)
		{
			QRectF signalRect = ts.tempDrawRect();
			lastDiscreteRect = signalRect;
			if (signalRect.isNull() == true)
			{
				break;
			}

			signalBackColor = (signalBackColor == drawParam.backColor1st()) ? drawParam.backColor2nd() : drawParam.backColor1st();
			painter->fillRect(signalRect, signalBackColor);
		}

		// Specific drawing for analog signals
		//
		Q_ASSERT(drawParam.viewMode() == E::TrendViewMode::Separated || drawParam.viewMode() == E::TrendViewMode::Overlapped);

		if (analogs.empty() == true && lastDiscreteRect.isEmpty() == false)
		{
			// Draw background in switched color, it is just nice to separate discretes from empty area
			//
			QRectF blankArea(lastDiscreteRect.bottomLeft(), insideRect.bottomRight());

			signalBackColor = (signalBackColor == drawParam.backColor1st()) ? drawParam.backColor2nd() : drawParam.backColor1st();
			painter->fillRect(blankArea, signalBackColor);
		}

		if (drawParam.viewMode() == E::TrendViewMode::Separated && analogs.empty() == false)
		{
			for (const TrendSignalParam& ts : analogs)
			{
				QRectF signalRect = ts.tempDrawRect();
				if (signalRect.isNull() == true)
				{
					break;
				}

				signalBackColor = (signalBackColor == drawParam.backColor1st()) ? drawParam.backColor2nd() : drawParam.backColor1st();
				painter->fillRect(signalRect, signalBackColor);
			}
		}

		if (drawParam.viewMode() == E::TrendViewMode::Overlapped && analogs.empty() == false)
		{
			QRectF signalRect = analogs.front().tempDrawRect();

			if (signalRect.isNull() == false)
			{
				signalBackColor = (signalBackColor == drawParam.backColor1st()) ? drawParam.backColor2nd() : drawParam.backColor1st();
				painter->fillRect(signalRect, signalBackColor);
			}
		}

		// Draw insideRect
		//
		QPen insideRectPen(Qt::darkGray, drawParam.cosmeticPenWidth(), Qt::SolidLine);
		painter->setPen(insideRectPen);
		painter->setBrush(Qt::BrushStyle::NoBrush);

		painter->drawRect(insideRect);

		return;
	}

	void TrendImpl::drawTimeGrid(QPainter* painter, const QRectF& laneRect, const QRectF& insideRect, const TrendParam& drawParam) const
	{
		double dpiX = drawParam.realDpiX();

		// Calc time grid
		//
		static const std::array<qint64, 31> possibleTimeGridIntervals = {
			5_ms,   10_ms,  20_ms,  25_ms,   50_ms,   100_ms,  200_ms,   250_ms,   500_ms,      1_sec,  2_sec,
			5_sec,  10_sec, 15_sec, 20_sec,  30_sec,  1_min,   90_sec,   2_min,    5_min,       10_min, 15_min,
			20_min, 30_min, 1_hour, 2_hours, 3_hours, 6_hours, 12_hours, 24_hours, 24_hours * 7};

		QString estimatedString = (drawParam.duration() < 10_sec) ? "HH:MM:SS.XXX" : "HH:MM:SS";
		auto estimatedStringSize = calcTextSize(painter, estimatedString, drawParam);

		double minTimeInterval = estimatedStringSize.width() * 1.2;

		TimeStamp startTimeStamp = drawParam.startTimeStamp();
		qint64 duration = drawParam.duration();

		qint64 timeGridInterval = possibleTimeGridIntervals[possibleTimeGridIntervals.size() - 1];
		double inchGridInterval = 0;

		for (qint64 possibleInterval : possibleTimeGridIntervals)
		{
			TimeStamp ct = TimeStamp{startTimeStamp.timeStamp + possibleInterval};

			double x = TrendScale::timeToScaledPixel(ct, insideRect, startTimeStamp, duration);

			if (x - insideRect.left() >= minTimeInterval)
			{
				timeGridInterval = possibleInterval;
				inchGridInterval = x - insideRect.left();
				break;
			}
		}

		// Align startGridPosition
		//
		TimeStamp startGrid = drawParam.startTimeStamp();

		startGrid.timeStamp /= timeGridInterval;
		startGrid.timeStamp *= timeGridInterval;

		// calc time grid positions
		//
		struct PosTimePair
		{
			double x{};
			TimeStamp timeStamp{};
		};

		int timeGridCount = static_cast<int>(insideRect.width() / inchGridInterval);
		if (timeGridCount < 0 || timeGridCount > 100)
		{
			return;
		}

		std::vector<PosTimePair> timeGridPos;
		timeGridPos.reserve(timeGridCount + 1);

		for (int i = 0; i < timeGridCount + 2; i++)
		{
			TimeStamp ct = TimeStamp{startGrid.timeStamp + i * timeGridInterval};
			double x = TrendScale::timeToScaledPixel(ct, insideRect, startTimeStamp, duration);

			// Make sure that x is proper aligned for nice look of cosmetic pen
			//
			x = static_cast<double>(static_cast<int>(x * dpiX)) / dpiX;

			if (x < insideRect.left())
			{
				continue;
			}

			if (x > insideRect.right())
			{
				break;
			}

			PosTimePair p;
			p.x = x;
			p.timeStamp = ct;

			timeGridPos.push_back(p);
		}

		// Draw time grid
		//
		QPen timeGridPen(Qt::lightGray, drawParam.cosmeticPenWidth(), Qt::PenStyle::DashLine);
		painter->setPen(timeGridPen);

		QDate lastDate;

		for (const PosTimePair& p : timeGridPos)
		{
			QPointF pt1(p.x, insideRect.top());
			QPointF pt2(p.x, insideRect.bottom());
			painter->drawLine(pt1, pt2);

			QTime time = p.timeStamp.toDateTime().time();
			QDate date = p.timeStamp.toDateTime().date();

			if (lastDate != date && time == QTime{0, 0, 0, 0})
			{
				lastDate = date;

				double x = static_cast<double>(p.x * dpiX + 1) / dpiX;

				QPointF linePt1{x, insideRect.top()};
				QPointF linePt2{x, insideRect.bottom()};
				painter->drawLine(linePt1, linePt2);
			}
		}

		// Draw text, time and date
		//
		if (drawParam.showTimeLabels() == true || drawParam.showDateLabels() == true)
		{
			QRectF textClipRect(laneRect.left(), insideRect.bottom(), laneRect.width(), laneRect.bottom() - insideRect.bottom());
			painter->setClipRect(textClipRect);

			painter->setPen(Qt::black);
			QString lastDateText;
			for (const PosTimePair& p : timeGridPos)
			{
				QDateTime dateTime = p.timeStamp.toDateTime();

				QString timeText =
					timeGridInterval < 1_sec ? DateTimeToString ::timeMs(dateTime.time()) : DateTimeToString::timeSec(dateTime.time());
				QRectF timeTextRect{p.x - 2.0,
									insideRect.bottom(),
									4.0,
									drawParam.showDateLabels() ? (laneRect.bottom() - insideRect.bottom()) / 2.0 :
																 (laneRect.bottom() - insideRect.bottom())};

				if (drawParam.showTimeLabels() == true)
				{
					drawText(painter, timeText, timeTextRect, drawParam, Qt::AlignCenter);
				}

				QString dateText = DateTimeToString::date(dateTime.date());
				QRectF dateTextRect{p.x - 2.0,
									drawParam.showTimeLabels() == true ? timeTextRect.bottom() : insideRect.bottom(),
									4.0,
									drawParam.showTimeLabels() == true ? (laneRect.bottom() - insideRect.bottom()) / 2.0 :
																		 (laneRect.bottom() - insideRect.bottom())};

				if (drawParam.showDateLabels() == true && lastDateText != dateText)
				{
					lastDateText = dateText;
					drawText(painter, dateText, dateTextRect, drawParam, Qt::AlignCenter);
				}
			}

			painter->setClipping(false);
		}

		// --
		//

		return;
	}

	void TrendImpl::drawSignalsDecor(QPainter* painter,
									 const Lane& lane,
									 const TrendParam& drawParam,
									 const std::vector<TrendSignalParam>& discretes,
									 const std::vector<TrendSignalParam>& analogs) const
	{
		Q_ASSERT(painter);
		painter->setClipRect(lane.laneRect);

		// Draw DISCRETE signal id, caption and scale ("0", "1")
		//
		for (const TrendSignalParam& ts : discretes)
		{
			QRectF signalRect = ts.tempDrawRect();
			if (signalRect.isNull() == true)
			{
				break;
			}

			QString signalText;
			if (drawParam.showSignalIds() == true && drawParam.showSignalCaptions() == true)
			{
				signalText = ts.archiveServerShortId().isEmpty() == true ?
								 QString("  %1 - %2").arg(ts.signalId()).arg(ts.caption()) :
								 QString("  %1 - %2 (%3)").arg(ts.signalId()).arg(ts.caption()).arg(ts.archiveServerShortId());
			}

			if (drawParam.showSignalIds() == true && drawParam.showSignalCaptions() == false)
			{
				signalText = ts.archiveServerShortId().isEmpty() == true ?
								 QString("  %1").arg(ts.signalId()) :
								 QString("  %1 (%2)").arg(ts.signalId()).arg(ts.archiveServerShortId());
			}

			if (drawParam.showSignalIds() == false && drawParam.showSignalCaptions() == true)
			{
				signalText = ts.archiveServerShortId().isEmpty() == true ?
								 QString("  %1").arg(ts.caption()) :
								 QString("  %1 (%2)").arg(ts.caption()).arg(ts.archiveServerShortId());
			}

			if (signalText.isEmpty() == false)
			{
				painter->setPen(ts.color());

				QRectF testDescriptionBoundRect;
				drawText(painter,
						 signalText,
						 signalRect,
						 drawParam,
						 Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine,
						 &testDescriptionBoundRect);

				auto scr = std::make_pair(ts.appSignalId(), testDescriptionBoundRect);
				drawParam.signalDescriptionRect().push_back(scr);
			}

			// Draw scale 0/1 for discretes.
			//
			if (drawParam.showSignalScales() == true)
			{
				QRectF scaleAreaRect = calcScaleAreaRect(lane.laneRect, signalRect);

				drawText(painter, "0 ", scaleAreaRect, drawParam, Qt::AlignRight | Qt::AlignBottom);
				drawText(painter, "1 ", scaleAreaRect, drawParam, Qt::AlignRight | Qt::AlignTop);
			}

			// Draw real-time mode last (current) value.
			//
			drawSignalsDecorRealtimeValue(painter, lane, signalRect, drawParam, ts);
		}

		// Draw ANALOG signal id, caption and scale for TrendView::Separated mode
		//
		Q_ASSERT(drawParam.viewMode() == E::TrendViewMode::Separated || drawParam.viewMode() == E::TrendViewMode::Overlapped);

		if (drawParam.viewMode() == E::TrendViewMode::Separated && analogs.empty() == false)
		{
			for (const TrendSignalParam& ts : analogs)
			{
				QRectF signalRect = ts.tempDrawRect();
				if (signalRect.isNull() == true)
				{
					break;
				}

				QString signalText;
				if (drawParam.showSignalIds() == true && drawParam.showSignalCaptions() == true)
				{
					if (ts.unit().isEmpty() == true)
					{
						signalText = ts.archiveServerShortId().isEmpty() == true ?
										 QString("  %1 - %2").arg(ts.signalId()).arg(ts.caption()) :
										 QString("  %1 - %2 (%3)").arg(ts.signalId()).arg(ts.caption()).arg(ts.archiveServerShortId());
					}
					else
					{
						signalText = ts.archiveServerShortId().isEmpty() == true ?
										 QString("  %1 - %2, %3").arg(ts.signalId()).arg(ts.caption()).arg(ts.unit()) :
										 QString("  %1 - %2, %3 (%4)")
											 .arg(ts.signalId())
											 .arg(ts.caption())
											 .arg(ts.unit())
											 .arg(ts.archiveServerShortId());
					}
				}

				if (drawParam.showSignalIds() == true && drawParam.showSignalCaptions() == false)
				{
					signalText = ts.archiveServerShortId().isEmpty() == true ?
									 QString("  %1").arg(ts.signalId()) :
									 QString("  %1 (%2)").arg(ts.signalId()).arg(ts.archiveServerShortId());
				}

				if (drawParam.showSignalIds() == false && drawParam.showSignalCaptions() == true)
				{
					if (ts.unit().isEmpty() == true)
					{
						signalText = ts.archiveServerShortId().isEmpty() == true ?
										 QString("  %1").arg(ts.caption()) :
										 QString("  %1 (%2)").arg(ts.caption()).arg(ts.archiveServerShortId());
					}
					else
					{
						signalText = ts.archiveServerShortId().isEmpty() == true ?
										 QString("  %1, %2").arg(ts.caption()).arg(ts.unit()) :
										 QString("  %1, %2 (%3)").arg(ts.caption()).arg(ts.unit()).arg(ts.archiveServerShortId());
					}
				}

				// Check the scale view limits
				//
				bool highLimitOk = false;
				bool lowLimitOk = false;

				double highLimit = TrendScale::scaleHighLimit(ts, drawParam.scaleType(), &highLimitOk);
				Q_UNUSED(highLimit);

				double lowLimit = TrendScale::scaleLowLimit(ts, drawParam.scaleType(), &lowLimitOk);
				Q_UNUSED(lowLimit);

				if (highLimitOk == false || lowLimitOk == false)
				{
					signalText += QObject::tr(" [can't render the trend, scale is not valid for current mode]");
				}

				if (signalText.isEmpty() == false)
				{
					painter->setPen(ts.color());

					// Draw description text
					//
					QRectF testDescriptionBoundRect;
					drawText(painter,
							 signalText,
							 signalRect,
							 drawParam,
							 Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine,
							 &testDescriptionBoundRect);

					auto scr = std::make_pair(ts.appSignalId(), testDescriptionBoundRect);
					drawParam.signalDescriptionRect().push_back(scr);
				}

				// Draw horizontal grid and scale
				//
				drawAnalogSignalsGridSeparateMode(painter, lane.laneRect, drawParam, ts, ts.color());

				// Draw real-time mode last value.
				//
				drawSignalsDecorRealtimeValue(painter, lane, signalRect, drawParam, ts);
			}
		}

		// Draw ANALOG signal id, caption and scale for TrendView::Overlapped mode
		//
		if (drawParam.viewMode() == E::TrendViewMode::Overlapped && analogs.empty() == false)
		{
			QRectF signalRect = analogs.front().tempDrawRect();

			for (const TrendSignalParam& ts : analogs)
			{
				QString signalText;
				if (drawParam.showSignalIds() == true && drawParam.showSignalCaptions() == true)
				{
					if (ts.unit().isEmpty() == true)
					{
						signalText = ts.archiveServerShortId().isEmpty() == true ?
										 QString("  %1 - %2").arg(ts.signalId()).arg(ts.caption()) :
										 QString("  %1 - %2 (%3)").arg(ts.signalId()).arg(ts.caption()).arg(ts.archiveServerShortId());
					}
					else
					{
						signalText = ts.archiveServerShortId().isEmpty() == true ?
										 QString("  %1 - %2, %3").arg(ts.signalId()).arg(ts.caption()).arg(ts.unit()) :
										 QString("  %1 - %2, %3 (%4)")
											 .arg(ts.signalId())
											 .arg(ts.caption())
											 .arg(ts.unit())
											 .arg(ts.archiveServerShortId());
					}
				}

				if (drawParam.showSignalIds() == true && drawParam.showSignalCaptions() == false)
				{
					signalText = ts.archiveServerShortId().isEmpty() == true ?
									 QString("  %1").arg(ts.signalId()) :
									 QString("  %1 (%2)").arg(ts.signalId()).arg(ts.archiveServerShortId());
				}

				if (drawParam.showSignalIds() == false && drawParam.showSignalCaptions() == true)
				{
					if (ts.unit().isEmpty() == true)
					{
						signalText = ts.archiveServerShortId().isEmpty() == true ?
										 QString("  %1").arg(ts.caption()) :
										 QString("  %1 (%2)").arg(ts.caption()).arg(ts.archiveServerShortId());
					}
					else
					{
						signalText = ts.archiveServerShortId().isEmpty() == true ?
										 QString("  %1, %2").arg(ts.caption()).arg(ts.unit()) :
										 QString("  %1, %2 (%3)").arg(ts.caption()).arg(ts.unit()).arg(ts.archiveServerShortId());
					}
				}

				// Check the scale view limits
				//
				bool highLimitOk = false;
				bool lowLimitOk = false;

				double highLimit = TrendScale::scaleHighLimit(ts, drawParam.scaleType(), &highLimitOk);
				Q_UNUSED(highLimit);

				double lowLimit = TrendScale::scaleLowLimit(ts, drawParam.scaleType(), &lowLimitOk);
				Q_UNUSED(lowLimit);

				if (highLimitOk == false || lowLimitOk == false)
				{
					signalText += QObject::tr(" [can't render the trend, scale is not valid for current mode]");
				}

				if (signalText.isEmpty() == false)
				{
					painter->setPen(ts.color());

					QRectF testDesctriptionBoundRect;
					drawText(painter,
							 signalText,
							 signalRect,
							 drawParam,
							 Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine,
							 &testDesctriptionBoundRect);

					auto scr = std::make_pair(ts.appSignalId(), testDesctriptionBoundRect);
					drawParam.signalDescriptionRect().push_back(scr);

					// Shift rect
					//
					signalRect.setTop(testDesctriptionBoundRect.bottom() + testDesctriptionBoundRect.height() * 0.25);
				}

				// Draw realtime mode last value.
				//
				drawSignalsDecorRealtimeValue(painter, lane, signalRect, drawParam, ts);
			}

			// Draw horizontal grid and scale
			//
			drawAnalogSignalsGridOverlappedMode(painter, lane.laneRect, drawParam, analogs);
		}

		//		// --
		//		//
		painter->setClipping(false);

		return;
	}

	void TrendImpl::drawSignalsDecorRealtimeValue(QPainter* painter,
												  const Lane& lane,
												  const QRectF& signalRect,
												  const TrendParam& drawParam,
												  const TrendSignalParam& signalParam) const
	{
		if (drawParam.trendMode() != E::TrendMode::Realtime)
		{
			return;
		}

		// Getting data without requesting if it is not present.
		//
		std::optional<TrendStateItem> lastStateOpt = signalSet().lastRealtimeState(signalParam.appSignalHash(), drawParam.timeType());

		// If there is no last state, then draw only the last lane value (sign ?).
		//
		bool definitelyDraw = false;

		if (lastStateOpt.has_value() == false)
		{
			// Corner case, there is no value at all, draw ? on the last lane only.
			//
			if (lane.index == drawParam.laneCount() - 1)
			{
				definitelyDraw = true;
			}
			else
			{
				return;
			}
		}
		else
		{
			// lastStateOpt.has_value() == true
			//
			TrendStateItem lastState = lastStateOpt.value();
			QDateTime ts = lastState.getTime(drawParam.timeType()).toDateTime();

			if (lane.index == 0 && ts < lane.startTime)
			{
				definitelyDraw = true;
			}

			QDateTime laneEndTime = lane.startTime.addMSecs(drawParam.duration());
			if (lane.index == drawParam.laneCount() - 1 && ts > laneEndTime)
			{
				definitelyDraw = true;
			}
		}

		// Do not draw lanes if the last point out of the lane.
		//
		if (definitelyDraw == false)
		{
			TrendStateItem lastState = lastStateOpt.value();

			QDateTime ts = lastState.getTime(drawParam.timeType()).toDateTime();
			QDateTime laneEndTime = lane.startTime.addMSecs(drawParam.duration());

			if (ts < lane.startTime || ts > laneEndTime)
			{
				return;
			}
		}

		// Form string value.
		//
		TrendStateItem lastState = lastStateOpt.value_or(TrendStateItem{});
		QString strValue;

		if (lastState.isValid() == true)
		{
			switch (signalParam.type())
			{
			case E::SignalType::Analog:
				{
					bool ok = true;
					/*double value = */ TrendScale::valueToScaleValue(lastState.value, drawParam.scaleType(), &ok);
					if (ok == false)
					{
						strValue = "?";
					}
					else
					{
						strValue = TrendScale::scaleValueText(lastState.value, drawParam.scaleType(), signalParam);
					}
				}
				break;
			case E::SignalType::Discrete:
				strValue = QString::number(lastState.value);
				break;
			default:
				strValue = QString::number(lastState.value);
			}
		}
		else
		{
			strValue = "?";
		}

		QString drawTextValue = QString{" %1 "}.arg(strValue);

		// Get bounding rect.
		//
		QRgb color = signalParam.color();
		painter->setPen(color);

		QRectF boundingRect;
		drawText(painter, drawTextValue, signalRect, drawParam, Qt::AlignRight | Qt::AlignTop | Qt::TextSingleLine, &boundingRect);

		boundingRect = QRectF{signalRect.right() - boundingRect.width(), signalRect.top(), boundingRect.width(), boundingRect.height()};

		// Draw real-time value background.
		//
		QColor semitransparentColor = drawParam.backColor2nd();
		semitransparentColor.setAlpha(150);

		QBrush fillRectBrush(semitransparentColor);
		painter->fillRect(boundingRect, fillRectBrush);

		// Draw real-time value text.
		//
		drawText(painter, drawTextValue, boundingRect, drawParam, Qt::AlignRight | Qt::AlignTop | Qt::TextSingleLine, nullptr);

		return;
	}

	void TrendImpl::drawAnalogSignalsGridSeparateMode(QPainter* painter,
													  const QRectF& laneRect,
													  const TrendParam& drawParam,
													  const TrendSignalParam& signal,
													  TrendColor color) const
	{
		Q_ASSERT(painter);
		painter->setClipping(false);

		QRectF signalRect = signal.tempDrawRect();
		QRectF scaleAreaRect = calcScaleAreaRect(laneRect, signalRect);

		if (signalRect.isEmpty() == true || scaleAreaRect.isEmpty() == true)
		{
			return;
		}

		bool ok = false;

		double highLimit = TrendScale::scaleHighLimit(signal, drawParam.scaleType(), &ok);
		if (ok == false)
		{
			return;
		}

		double lowLimit = TrendScale::scaleLowLimit(signal, drawParam.scaleType(), &ok);
		if (ok == false)
		{
			return;
		}

		if (std::fabs(highLimit - lowLimit) <= std::numeric_limits<double>::min())
		{
			// Divide by 0 possible
			//
			return;
		}

		// Get grid values
		//
		double minInchInterval = 1.0 / 4.0;                          // 1/4 in -- minimum inches interval

		auto scaleValues = TrendScale::scaleValues(drawParam.scaleType(),
												   lowLimit,
												   highLimit,
												   signalRect,
												   minInchInterval); // first: value, second: display value
		if (scaleValues.has_value() == false)
		{
			return;
		}

		// Draw horz grids
		//
		double dpiY = drawParam.realDpiY();

		QPen gridPen(Qt::lightGray, drawParam.cosmeticPenWidth(), Qt::PenStyle::DashLine);
		painter->setPen(gridPen);

		std::vector<std::pair<double, double>> grids; // first: y pos, second: display value
		grids.reserve(scaleValues->size());

		for (const std::pair<double, double>& p : *scaleValues)
		{
			double value = p.first;

			double y = TrendScale::valueToScaledPixel(value, signalRect, lowLimit, highLimit);
			double antiAliasedY = static_cast<double>(static_cast<int>(y * dpiY)) / dpiY; // Align to DPI

			if (antiAliasedY < signalRect.top() || antiAliasedY > signalRect.bottom())
			{
				continue;
			}

			painter->drawLine(QPointF(signalRect.left(), antiAliasedY), QPointF(signalRect.right(), antiAliasedY));

			double scaleValue = p.second;

			grids.emplace_back(y, scaleValue);
		}

		if (drawParam.showSignalScales() == false)
		{
			return;
		}

		// Draw grid values
		//
		painter->setPen(color);

		const auto boundTextSize = calcTextSize(painter, QStringLiteral("0"), drawParam);
		const double textHeight = boundTextSize.height();

		painter->setClipRect(scaleAreaRect);

		for (const std::pair<double, double>& p : grids)
		{
			double y = p.first;
			double value = p.second;

			QRectF textRect(scaleAreaRect.left(), y - textHeight / 2.0, scaleAreaRect.width(), textHeight);

			if (textRect.top() < scaleAreaRect.top() || textRect.bottom() > scaleAreaRect.bottom())
			{
				continue;
			}

			QString text = TrendScale::scaleValueText(value, drawParam.scaleType(), signal);

			drawText(painter, text, textRect, drawParam, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextDontClip);
		}

		painter->setClipping(false);
		return;
	}

	void TrendImpl::drawAnalogSignalsGridOverlappedMode(QPainter* painter,
														const QRectF& laneRect,
														const TrendParam& drawParam,
														const std::vector<TrendSignalParam>& analogs) const
	{
		Q_ASSERT(painter);
		painter->setClipping(false);

		if (analogs.empty() == true)
		{
			return;
		}

		int columnCount = calcAnalogScaleColumnCount(analogs, drawParam);
		int rowCount = static_cast<int>(analogs.size() / columnCount + (analogs.size() % columnCount));
		rowCount = std::clamp<int>(rowCount, 1, 4);

		if (columnCount == 1)
		{
			auto sameSignalLimitsPred = [scaleType = drawParam.scaleType(), &firstSignal = analogs[0]](const TrendSignalParam& sp)
			{
				return sp.viewLowLimit(scaleType) == firstSignal.viewLowLimit(scaleType) &&   // Same low limit
					   sp.viewHighLimit(scaleType) == firstSignal.viewHighLimit(scaleType) && // Same high limit
					   sp.analogFormat() == firstSignal.analogFormat() &&                     // Same format
					   sp.precision() == firstSignal.precision();                             // Same precision
			};

			if (analogs.size() == 1 || std::ranges::all_of(analogs, sameSignalLimitsPred) == true)
			{
				auto color = analogs.size() == 1 ? analogs[0].color() : 0;
				drawAnalogSignalsGridSeparateMode(painter, laneRect, drawParam, analogs[0], color);
				return;
			}
		}

		QRectF signalRect = analogs[0].tempDrawRect();
		QRectF scaleAreaRect = calcScaleAreaRect(laneRect, signalRect);
		if (signalRect.isEmpty() == true || scaleAreaRect.isEmpty() == true)
		{
			return;
		}

		bool ok1 = false;
		bool ok2 = false;

		const double highLimit = TrendScale::scaleHighLimit(analogs[0], drawParam.scaleType(), &ok1);
		const double lowLimit = TrendScale::scaleLowLimit(analogs[0], drawParam.scaleType(), &ok2);
		if (ok1 == false || ok2 == false)
		{
			return;
		}

		if (std::fabs(highLimit - lowLimit) <= std::numeric_limits<double>::min())
		{
			// Divide by 0 possible
			//
			return;
		}

		// Get grid values
		//
		const std::array minVertIntervals = {4.0 / 8.0,
											 3.0 / 8.0,
											 3.0 / 5.0,
											 4.0 / 5.0}; // minimum inches interval for 1-2, 3-6, 7+ signals
		const double minInchInterval = minVertIntervals[std::clamp(rowCount - 1, 0, static_cast<int>(minVertIntervals.size() - 1))];

		auto scaleValues = TrendScale::scaleValues(drawParam.scaleType(),
												   lowLimit,
												   highLimit,
												   signalRect,
												   minInchInterval); // first: value, second: display value
		if (scaleValues.has_value() == false)
		{
			return;
		}

		// Draw horz grids
		//
		const double dpiY = drawParam.realDpiY();

		QPen gridPen(Qt::lightGray, drawParam.cosmeticPenWidth(), Qt::PenStyle::DashLine);
		painter->setPen(gridPen);

		std::vector<std::pair<double, double>> grids; // first: y pos, second: display value
		grids.reserve(scaleValues->size());

		for (const std::pair<double, double>& p : *scaleValues)
		{
			double value = p.first;
			double y = TrendScale::valueToScaledPixel(value, signalRect, lowLimit, highLimit);
			double antiAliasedY = static_cast<double>(static_cast<int>(y * dpiY)) / dpiY; // Align to DPI

			if (antiAliasedY < signalRect.top() || antiAliasedY > signalRect.bottom())
			{
				continue;
			}

			painter->drawLine(QPointF(signalRect.left(), antiAliasedY), QPointF(signalRect.right(), antiAliasedY));

			double scaleValue = p.second;

			grids.emplace_back(y, scaleValue);
		}

		if (drawParam.showSignalScales() == false)
		{
			return;
		}

		const auto boundTextSize = calcTextSize(painter, QStringLiteral("0"), drawParam);
		const double textHeight = boundTextSize.height();

		painter->setClipRect(scaleAreaRect);

		auto drawTextFunc = [&scaleAreaRect, textHeight, painter](double gridY,
																  double value,
																  const TrendParam& drawParam,
																  const TrendSignalParam& signal,
																  int column,
																  int row,
																  int columnCount,
																  int rowCount)
		{
			assert(rowCount >= 2 && rowCount <= 4);

			double cellWidth = scaleAreaRect.width() / columnCount;
			double cellHeight = textHeight;
			double cellX = scaleAreaRect.left() + column * cellWidth;
			double cellY = gridY + (-textHeight * rowCount / 2.0) + row * cellHeight;

			QRectF textRect(cellX, cellY, cellWidth, cellHeight);
			if (textRect.top() < scaleAreaRect.top() || textRect.bottom() > scaleAreaRect.bottom())
			{
				return;
			}

			painter->setPen(signal.color());

			QString text = TrendScale::scaleValueText(value, drawParam.scaleType(), signal);
			drawText(painter, text, textRect, drawParam, Qt::AlignHCenter | Qt::AlignVCenter);
			return;
		};

		auto getGridsForSignalFunc = [&grids, &signalRect](const TrendSignalParam& signal,
														   const TrendParam& drawParam) -> std::vector<std::pair<double, double>>
		{
			std::vector<std::pair<double, double>> res;
			bool ok1 = false;
			bool ok2 = false;

			double signalHighLimit = TrendScale::scaleHighLimit(signal, drawParam.scaleType(), &ok1);
			double signalLowLimit = TrendScale::scaleLowLimit(signal, drawParam.scaleType(), &ok2);
			if (ok1 == false || ok2 == false || std::fabs(signalHighLimit - signalLowLimit) <= std::numeric_limits<double>::min())
			{
				return res;
			}

			for (const auto& [gridY, gridValue] : grids)
			{
				double scaleValue = TrendScale::scaledPixelToValue(gridY, signalRect, signalLowLimit, signalHighLimit);
				double value = TrendScale::valueFromScaleValue(scaleValue, drawParam.scaleType(), &ok1);
				if (ok1 == false)
				{
					continue;
				}

				res.emplace_back(gridY, value);
			}

			return res;
		};

		auto drawSignalScale = [&getGridsForSignalFunc,
								&drawTextFunc,
								&drawParam](const TrendSignalParam& signal, int column, int row, int columnCount, int rowCount)
		{
			auto gridsForSignal = getGridsForSignalFunc(signal, drawParam);
			for (const auto [gridY, value] : gridsForSignal)
			{
				drawTextFunc(gridY, value, drawParam, signal, column, row, columnCount, rowCount);
			}
		};

		// Draw scales for signals in columns and rows
		//
		{
			int signalIndex = 0;
			for (const TrendSignalParam& ts : analogs | std::views::take(8))
			{
				drawSignalScale(ts, signalIndex / rowCount, signalIndex % rowCount, columnCount, rowCount);
				signalIndex++;
			}
		}

		painter->setClipping(false);
		return;
	}

	void TrendImpl::drawSignalTrend(QPainter* painter,
									const TrendSignalParam& signal,
									const TrendParam& drawParam,
									std::stop_token stoken) const
	{
		Q_ASSERT(painter);

		QRectF signalRect = signal.tempDrawRect();
		if (signalRect.isEmpty() == true)
		{
			return;
		}

		// Get signal data
		//
		TimeStamp requestStartTime(drawParam.startTimeStamp().timeStamp - 1_hour);
		TimeStamp requestFinishTime(drawParam.startTimeStamp().timeStamp + drawParam.duration() + 2_hours);

		QDateTime startTime = requestStartTime.toDateTime();
		QDateTime finishTime = requestFinishTime.toDateTime();

		std::list<std::shared_ptr<const OneHourData>> signalData;
		bool requestResult = false;

		if (drawParam.trendDataProvider() != nullptr)
		{
			requestResult =
				drawParam.trendDataProvider()
					->trendData(uuid(), signal, startTime, finishTime, drawParam.timeType(), drawParam.trendMode(), &signalData);

			if (requestResult == false)
			{
				signalData.clear();
			}
		}

		// --
		//
		if (signal.isDiscrete() == true)
		{
			// signalData will release some data inside, after drawing
			//
#ifdef EXPERIMENTAL_TREND_DRAW_WITH_BARS
			if ((QApplication::keyboardModifiers() & Qt::CTRL) && (QApplication::keyboardModifiers() & Qt::SHIFT))
			{
				drawSignalTrendDiscrete(painter, signal, drawParam, signalData, stoken);
			}
			else
			{
				drawSignalTrendDiscreteBars(painter, signal, drawParam, signalData, stoken);
			}
#else
			drawSignalTrendDiscrete(painter, signal, drawParam, signalData, stoken);
#endif
		}

		if (signal.isAnalog() == true)
		{
#ifdef EXPERIMENTAL_TREND_DRAW_WITH_BARS
			if ((QApplication::keyboardModifiers() & Qt::CTRL) && (QApplication::keyboardModifiers() & Qt::SHIFT))
			{
				drawSignalTrendAnalog(painter, signal, drawParam, signalData, stoken);
			}
			else
			{
				drawSignalTrendAnalogBars(painter, signal, drawParam, signalData, stoken);
			}
#else
			// signalData will release some data inside, after drawing
			//
			drawSignalTrendAnalog(painter, signal, drawParam, signalData, stoken);
#endif
		}

		// --
		//
		return;
	}

	void TrendImpl::drawSignalTrendDiscrete(QPainter* painter,
											const TrendSignalParam& signal,
											const TrendParam& drawParam,
											std::list<std::shared_ptr<const OneHourData>>& signalData,
											std::stop_token stoken) const
	{
		Q_ASSERT(painter);
		Q_ASSERT(signal.isDiscrete() == true);

		QRectF signalRect = signal.tempDrawRect();

		// Set clip region
		//
		painter->setClipRect(signalRect);

		// Set pen to get font metrics
		//
		QPen linePen({signal.color()},
					 (signal.lineWeight() <= 1.0) ? drawParam.cosmeticPenWidth() : signal.lineWeight() / drawParam.realDpiY(),
					 Qt::SolidLine);
		painter->setPen(linePen);

		const auto textBoundSize = calcTextSize(painter, QStringLiteral("0"), drawParam);

		E::TimeType timeType = drawParam.timeType();

		const int recommendedSize = 8192 * 2;
		std::vector<QPointF> lines;
		lines.reserve(recommendedSize);

		TimeStamp startTimeStamp = drawParam.startTimeStamp();
		qint64 duration = drawParam.duration();

		double dpiY = drawParam.realDpiY();
		// double dpiX = drawParam.realDpiX();

		double yPos0 = signalRect.bottom() - textBoundSize.height() / 2.0;
		double yPos1 = signalRect.top() + textBoundSize.height() * 1.1;
		yPos0 =
			static_cast<double>(static_cast<int>(yPos0 * dpiY)) / dpiY; // Make sure that Y is proper aligned for nice look of cosmetic pen
		yPos1 =
			static_cast<double>(static_cast<int>(yPos1 * dpiY)) / dpiY; // Make sure that Y is proper aligned for nice look of cosmetic pen

		double rectLeft = signalRect.left();
		double rectRight = signalRect.right();

		double lastX = 0;
		double lastY = 0;

		// int pointIndex = 0;

		for (std::shared_ptr<const OneHourData>& hour : signalData) // REFERENCE! To actually free memory after use
		{
			if (stoken.stop_requested() == true)
			{
				break;
			}

#ifdef TREND_ZERO_COPY_TREND_DATA
			std::shared_lock lock{hour->mutex};
#endif
			for (const TrendStateRecord& record : hour->data_)
			{
				if (stoken.stop_requested() == true)
				{
					break;
				}

				for (const TrendStateItem& state : record.states)
				{
					TimeStamp ct = state.getTime(timeType);

					// Break line if it is not valid point
					//
					if (state.isValid() == false)
					{
						if (lines.empty() == false)
						{
							drawPolyline(painter, lines, signalRect);
							lines.clear();
						}

						continue;
					}

					double x = TrendScale::timeToScaledPixel(ct, signalRect, startTimeStamp, duration);
					// x = static_cast<double>(static_cast<int>(x * dpiX)) / dpiX;		// Make sure that X is proper aligned for nice look
					// of cosmetic pen

					double y = (state.value == 0) ? yPos0 : yPos1;

					// painter->fillRect(QRectF(x - 1.0/64.0, y - 1.0/64.0, 1.0/32.0, 1.0/32.0), signal.color());
					// drawText(painter, QString("%1").arg(pointIndex), QRectF(x - 1.0/64.0, y - 1.0/64.0, 1.0/32.0, 1.0/32.0), drawParam,
					// Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip); qDebug() << "pointIndex: " << pointIndex; pointIndex ++;

					if (lines.empty() == true)
					{
						lines.push_back(QPointF{x, y});

						lastX = x;
						lastY = y;
					}
					else
					{
						if (x != lastX || y != lastY) // If prev point the same, don't add this point
						{
							if (lastY == y)
							{
								lines.push_back(QPointF{x, y});
							}
							else
							{
								lines.push_back(QPointF{x, lastY});
								lines.push_back(QPointF{x, y});
							}

							lastX = x;
							lastY = y;
						}
					}

					// Check if the last points are before the left edge. If so, then compress them to one point.
					//
					if (lines.size() >= 2)
					{
						const QPointF& p1 = lines[lines.size() - 2];
						QPointF p2 = lines[lines.size() - 1]; // Do not make it ref, as lines.clear() will lead to the dangling reference.

						if (p1.x() < rectLeft && p2.x() < rectLeft)
						{
							lines.clear();
							lines.push_back(p2);
						}
					}

					if (lastX >= rectRight)
					{
						break; // end of drawing
					}
				} // for (const TrendStateItem& state : record.states)

				if (lines.size() >= recommendedSize)
				{
					drawPolyline(painter, lines, signalRect);

					QPointF lastPoint = lines.back();
					lines.clear();
					lines.push_back(lastPoint);
				}

				if (lastX >= rectRight)
				{
					break; // end of drawing
				}
			}

			if (lastX >= rectRight)
			{
				break;     // end of drawing
			}

			hour.reset();  // Free memory, we don't need it anymore
		}

		if (lines.size() >= 2)
		{
			drawPolyline(painter, lines, signalRect);
			lines.clear();
		}

		// Reset clipping
		//
		painter->setClipping(false);

		return;
	}

	void TrendImpl::drawSignalTrendDiscreteBars(QPainter* painter,
												const TrendSignalParam& signal,
												const TrendParam& drawParam,
												std::list<std::shared_ptr<const OneHourData>>& signalData,
												std::stop_token stoken) const
	{
		Q_ASSERT(painter);
		Q_ASSERT(signal.isDiscrete() == true);

		QRectF signalRect = signal.tempDrawRect();
		double barWidth = 1.0 / drawParam.realDpiX(); // 1 pixel width
		size_t totalBars = static_cast<size_t>(std::ceil(signalRect.width() / barWidth)) + 1;

		struct Bar
		{
			bool hasValue;
			bool has0;
			bool has1;

			void update(bool value)
			{
				if (hasValue == false)
				{
					hasValue = true;
					has0 = !value;
					has1 = value;
				}
				else
				{
					has0 = has0 || !value;
					has1 = has1 || value;
				}
			}
		};

		thread_local std::vector<Bar> bars;
		bars.clear();
		bars.resize(totalBars, Bar{});

		// Set clip region
		//
		painter->setClipRect(signalRect);

		// Set pen to get font metrics.
		//
		QPen linePen({signal.color()},
					 (signal.lineWeight() <= 1.0) ? drawParam.cosmeticPenWidth() : signal.lineWeight() / drawParam.realDpiY(),
					 Qt::SolidLine);
		painter->setPen(linePen);

		const auto textBoundSize = calcTextSize(painter, QStringLiteral("0"), drawParam);

		const E::TimeType timeType = drawParam.timeType();

		const TimeStamp startTimeStamp = drawParam.startTimeStamp();
		const qint64 duration = drawParam.duration();

		const double dpiY = drawParam.realDpiY();
		const double dpiX = drawParam.realDpiX();

		double yPos0 = signalRect.bottom() - textBoundSize.height() / 2.0;
		double yPos1 = signalRect.top() + textBoundSize.height() * 1.1;
		yPos0 =
			static_cast<double>(static_cast<int>(yPos0 * dpiY)) / dpiY; // Make sure that Y is proper aligned for nice look of cosmetic pen
		yPos1 =
			static_cast<double>(static_cast<int>(yPos1 * dpiY)) / dpiY; // Make sure that Y is proper aligned for nice look of cosmetic pen

		std::optional<TrendStateItem> prevState;

		for (std::shared_ptr<const OneHourData>& hour : signalData)     // REFERENCE! To actually free memory after use
		{
			if (stoken.stop_requested() == true)
			{
				break;
			}

#ifdef TREND_ZERO_COPY_TREND_DATA
			std::shared_lock lock{hour->mutex};
#endif
			for (const TrendStateRecord& record : hour->data_)
			{
				if (stoken.stop_requested() == true)
				{
					break;
				}

				for (const TrendStateItem& state : record.states)
				{
					if (state.isValid() == false)
					{
						prevState.reset();
						continue;
					}

					if (prevState.has_value() == false)
					{
						prevState = state;
						continue;
					}

					// We have a prev state and current state, draw bars between them on the level of [currentBarValue : prevState.value].
					// Also add a bar at the [prevState.value : state.value]

					TimeStamp currentTime = state.getTime(timeType);
					TimeStamp prevTime = prevState->getTime(timeType);

					double startX = TrendScale::timeToScaledPixel(prevTime, signalRect, startTimeStamp, duration);
					double endX = TrendScale::timeToScaledPixel(currentTime, signalRect, startTimeStamp, duration);
					double dx = endX - startX;
					size_t barCount = static_cast<size_t>(std::ceil(std::fabs(dx) / barWidth));

					auto barOffset = static_cast<std::ptrdiff_t>(std::floor((startX - signalRect.left()) / barWidth));
					if (barOffset + static_cast<std::ptrdiff_t>(barCount) < 0)
					{
						// This point is completely out of the left edge, skip it
						//
						prevState = state;
						continue;
					}

					const bool prevStateValue = static_cast<bool>(prevState->value);
					for (std::size_t cnt = 0; cnt < barCount; cnt++)
					{
						std::ptrdiff_t index = barOffset + static_cast<std::ptrdiff_t>(cnt);
						if (index < 0)
						{
							continue;
						}

						if (static_cast<size_t>(index) >= totalBars)
						{
							// Early exit if we are out of the right edge, no need to process more bars
							//
							break;
						}

						bars[index].update(prevStateValue);
					}

					// Add a bar at the last point with the value of the current state
					//
					if (size_t lastBarIndex = barOffset + barCount - 1; //
						lastBarIndex < bars.size())
					{
						bars[lastBarIndex].update(state.value);
					}

					// --
					//
					prevState = state;
				} // for (const TrendStateItem& state : record.states)
			}

#ifdef TREND_ZERO_COPY_TREND_DATA
#else
			hour.reset(); // Free memory, we don't need it anymore
#endif
		}

		// Draw bars
		//
#if 0
		{
			painter->setPen(Qt::NoPen);

			const double lineWeightFactor = (signal.lineWeight() <= 1.0) ? 1.0 : signal.lineWeight();
			const double minBarWidth = (1.0 / dpiX) * lineWeightFactor;
			const double minBarHeight = (1.0 / dpiY) * lineWeightFactor;

			QRectF barRect;

			for (size_t i = 0; i < bars.size(); i++)
			{
				const Bar& bar = bars[i];
				
				if (bar.hasValue == true)
				{
					double x = signalRect.left() + i * barWidth; // - minBarWidth / 2.0;
					double y = bar.min;
					double height = bar.max - bar.min;

					if (height == 0 || height < minBarHeight)
					{
						height = minBarHeight;
					}

					// If y and height are the same as the previous bar, just extend the width of the previous bar
					//
					if (barRect.isNull() == true)
					{
						barRect = QRectF(x, y, minBarWidth, height);
					}
					else 
					{
						if (std::fabs(barRect.top() - y) <= std::numeric_limits<double>::epsilon() &&
							std::fabs(barRect.height() - height) <= std::numeric_limits<double>::epsilon())
						{
							barRect.setWidth((x + minBarWidth) - barRect.left());
						}
						else
						{
							painter->fillRect(barRect, signal.color());
							barRect = QRectF(x, y, minBarWidth, height);
						}
					}
				}
				else
				{
					if (barRect.isNull() == false) 
					{
						painter->fillRect(barRect, signal.color());
						barRect = QRectF{};
					}
				}
			}

			if (barRect.isNull() == false)
			{
				painter->fillRect(barRect, signal.color());
				barRect = QRectF{};
			}
		}
#else
		{
			painter->setPen(Qt::NoPen);
			// painter->setRenderHint(QPainter::Antialiasing, false);

			const double cosmeticPenWidth = drawParam.cosmeticPenWidth() ? drawParam.cosmeticPenWidth() : (1.0 / dpiX);
			const double cosmeticPenHeight = drawParam.cosmeticPenWidth() ? drawParam.cosmeticPenWidth() : (1.0 / dpiY);
			const double signalLineWeight = (signal.lineWeight() == 0.0) ? 1 : signal.lineWeight();
			const double minBarWidth = signalLineWeight * cosmeticPenWidth;
			const double minBarHeight = signalLineWeight * cosmeticPenHeight;
			const double pixelWidth = 1.0 / dpiX;

			const double fullHeight = yPos0 - yPos1 + minBarHeight;
			const double drawPos0 = yPos0 - minBarHeight / 2.0;
			const double drawPos1 = yPos1 - minBarHeight / 2.0;

			auto color = signal.color();

			for (size_t i = 0; i < bars.size(); i++)
			{
				Bar bar = bars[i];

				if (bar.hasValue == true)
				{
					double x = signalRect.left() + i * barWidth + pixelWidth - minBarWidth / 2.0;

					double y;
					double h;

					if (bar.has0 && !bar.has1)
					{
						// Bottom
						//
						y = drawPos0;
						h = minBarHeight;
					}
					else if (!bar.has0 && bar.has1)
					{
						// Top
						//
						y = drawPos1;
						h = minBarHeight;
					}
					else if (bar.has0 && bar.has1)
					{
						// Vertical full bar
						//
						y = drawPos1;
						h = fullHeight;
					}
					else
					{
						assert(false);
						y = 0;
						h = 0;
					}

					painter->fillRect(QRectF{x, y, minBarWidth, h}, color);
				}
			}

			// painter->setRenderHint(QPainter::Antialiasing, true);
		}
#endif

		// Reset clipping
		//
		painter->setClipping(false);

		return;
	}

	void TrendImpl::drawSignalTrendAnalog(QPainter* painter,
										  const TrendSignalParam& signal,
										  const TrendParam& drawParam,
										  std::list<std::shared_ptr<const OneHourData>>& signalData,
										  std::stop_token stoken) const
	{
		Q_ASSERT(painter);
		Q_ASSERT(signal.isAnalog() == true);

		QRectF signalRect = signal.tempDrawRect();

		// Set clip region
		//
		painter->setClipRect(signalRect);

		painter->setRenderHint(QPainter::Antialiasing, false);

		// Draw trend
		//
		bool ok = false;

		double highLimit = TrendScale::scaleHighLimit(signal, drawParam.scaleType(), &ok);
		if (ok == false)
		{
			return;
		}

		double lowLimit = TrendScale::scaleLowLimit(signal, drawParam.scaleType(), &ok);
		if (ok == false)
		{
			return;
		}

		if (std::fabs(highLimit - lowLimit) <= std::numeric_limits<double>::min())
		{
			// Divide by 0 possible
			//
			return;
		}

		E::TimeType timeType = drawParam.timeType();

		QPen linePen({signal.color()},
					 (signal.lineWeight() <= 1.0) ? drawParam.cosmeticPenWidth() : signal.lineWeight() / drawParam.realDpiY());
		painter->setPen(linePen);

		const int recommendedSize = 8192 * 2;
		std::vector<QPointF> lines;
		lines.reserve(recommendedSize);

		TimeStamp startTimeStamp = drawParam.startTimeStamp();
		qint64 duration = drawParam.duration();

		double rectLeft = signalRect.left();
		double rectRight = signalRect.right();

		double lastX = 0;
		double lastY = 0;

		//		int pointIndex = 0;

		for (std::shared_ptr<const OneHourData>& hour : signalData) // REFERENCE! To actually free memory after use
		{
			if (stoken.stop_requested() == true)
			{
				break;
			}

#ifdef TREND_ZERO_COPY_TREND_DATA
			std::shared_lock lock{hour->mutex};
#endif
			for (const TrendStateRecord& record : hour->data_)
			{
				if (stoken.stop_requested() == true)
				{
					break;
				}

				for (const TrendStateItem& state : record.states)
				{
					// Break line if it is not valid point or value has wrong value (e.g. logarithm from negative)
					//
					if (state.isValid() == false || ok == false)
					{
						if (lines.empty() == false)
						{
							drawPolyline(painter, lines, signalRect);
							lines.clear();
						}

						continue;
					}

					// --
					//
					const TimeStamp& ct = state.getTime(timeType);
					double value = TrendScale::valueToScaleValue(state.value, drawParam.scaleType(), &ok);

					double x = TrendScale::timeToScaledPixel(ct, signalRect, startTimeStamp, duration);
					double y = TrendScale::valueToScaledPixel(value, signalRect, lowLimit, highLimit);

					//					painter->fillRect(QRectF(x - 1.0/64.0, y - 1.0/64.0, 1.0/32.0, 1.0/32.0), signal.color());
					//					drawText(painter, QString("%1").arg(pointIndex), QRectF(x - 1.0/64.0, y
					//- 1.0/64.0, 1.0/32.0, 1.0/32.0), drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip);
					// qDebug() << "DEBUG: Discrete draw pointIndex:" << pointIndex
					//							 << ", Flags: " << state.flags
					//							 << ", value: " << state.value
					//							 << ", timestamp: " << ct.toDateTime().toString("HH:mm:ss.zzz");
					//					pointIndex ++;

					if (lines.empty() == true)
					{
						lines.push_back(QPointF(x, y));
						lastX = x;
						lastY = y;
					}
					else
					{
						if (x != lastX || y != lastY) // If prev point is the same, don't add this point
						{
							if (lastY == y)
							{
								if (lines.size() >= 2 && lines[lines.size() - 2].y() == lastY)
								{
									// Just extend the last line
									//
									lines.back().rx() = x;
								}
								else
								{
									lines.push_back(QPointF(x, y));
								}
							}
							else
							{
								// Create another curve on line
								//
								lines.push_back(QPointF(x, lastY));
								lines.push_back(QPointF(x, y));
							}

							lastX = x;
							lastY = y;
						}
					}

					// Check if the last points are before the left edge. If so, then compress them to one point.
					//
					if (lines.size() >= 2)
					{
						const QPointF& p1 = lines[lines.size() - 2];
						QPointF p2 = lines[lines.size() - 1]; // Do not make it ref, as lines.clear() will lead to the dangling reference.

						if (p1.x() < rectLeft && p2.x() < rectLeft)
						{
							lines.clear();
							lines.push_back(p2);
						}
					}

					if (lastX >= rectRight)
					{
						break; // end of drawing
					}
				} // for (const TrendStateItem& state : record.states)

				if (lines.size() >= recommendedSize)
				{
					drawPolyline(painter, lines, signalRect);
					QPointF lastPoint = lines.back();
					lines.clear();
					lines.push_back(lastPoint);
				}

				if (lastX >= rectRight)
				{
					break; // end of drawing
				}
			} // for (const TrendStateRecord& record : data)

			if (lastX >= rectRight)
			{
				break;    // end of drawing
			}

			hour.reset(); // Free memory, we don't need it anymore
		}

		if (lines.size() >= 2)
		{
			drawPolyline(painter, lines, signalRect);
		}

		// Reset clipping
		//
		painter->setClipping(false);

		painter->setRenderHint(QPainter::Antialiasing, true);
		return;
	}

	void TrendImpl::drawSignalTrendAnalogBars(QPainter* painter,
											  const TrendSignalParam& signal,
											  const TrendParam& drawParam,
											  std::list<std::shared_ptr<const OneHourData>>& signalData,
											  std::stop_token stoken) const
	{
		Q_ASSERT(painter);
		Q_ASSERT(signal.isAnalog() == true);

		QRectF signalRect = signal.tempDrawRect();
		double barWidth = 1.0 / drawParam.realDpiX(); // 1 pixel width
		size_t totalBars = static_cast<size_t>(std::ceil(signalRect.width() / barWidth)) + 1;

		struct Bar
		{
			double min;
			double max;
			bool hasValue;
			void update(double value)
			{
				if (hasValue == false)
				{
					hasValue = true;
					min = value;
					max = value;
				}
				else
				{
					if (value < min)
					{
						min = value;
					}
					if (value > max)
					{
						max = value;
					}
				}
			}
		};

		thread_local std::vector<Bar> bars;
		bars.clear();
		bars.resize(totalBars, Bar{});

		// Set clip region
		//
		painter->setClipRect(signalRect);
		painter->setRenderHint(QPainter::Antialiasing, false);

		// Set pen to get font metrics.
		//
		QPen linePen({signal.color()},
					 (signal.lineWeight() <= 1.0) ? drawParam.cosmeticPenWidth() : signal.lineWeight() / drawParam.realDpiY());
		painter->setPen(linePen);

		// Draw trend
		//
		bool ok1 = false;
		bool ok2 = false;

		double highLimit = TrendScale::scaleHighLimit(signal, drawParam.scaleType(), &ok1);
		double lowLimit = TrendScale::scaleLowLimit(signal, drawParam.scaleType(), &ok2);
		if (ok1 == false || ok2 == false)
		{
			painter->setClipping(false);
			painter->setRenderHint(QPainter::Antialiasing, true);
			return;
		}

		if (std::fabs(highLimit - lowLimit) <= std::numeric_limits<double>::min())
		{
			// Divide by 0 possible
			//
			painter->setClipping(false);
			painter->setRenderHint(QPainter::Antialiasing, true);
			return;
		}

		const E::TimeType timeType = drawParam.timeType();

		TimeStamp startTimeStamp = drawParam.startTimeStamp();
		qint64 duration = drawParam.duration();
		std::optional<TrendStateItem> prevState;

		for (std::shared_ptr<const OneHourData>& hour : signalData) // REFERENCE! To actually free memory after use
		{
			if (stoken.stop_requested() == true)
			{
				break;
			}

#ifdef TREND_ZERO_COPY_TREND_DATA
			std::shared_lock lock{hour->mutex};
#endif
			for (const TrendStateRecord& record : hour->data_)
			{
				if (stoken.stop_requested() == true)
				{
					break;
				}

				bool ok = false;
				for (const TrendStateItem& state : record.states)
				{
					// Break line if it is not valid point or value has wrong value (e.g. logarithm from negative)
					//
					if (state.isValid() == false || ok == false)
					{
						prevState.reset();
						continue;
					}

					if (prevState.has_value() == false)
					{
						prevState = state;
						continue;
					}

					// --
					//
					TimeStamp currentTime = state.getTime(timeType);
					TimeStamp prevTime = prevState->getTime(timeType);

					double startX = TrendScale::timeToScaledPixel(prevTime, signalRect, startTimeStamp, duration);
					double endX = TrendScale::timeToScaledPixel(currentTime, signalRect, startTimeStamp, duration);
					double dx = endX - startX;
					size_t barCount = static_cast<size_t>(std::ceil(std::fabs(dx) / barWidth));

					auto barOffset = static_cast<std::ptrdiff_t>(std::floor((startX - signalRect.left()) / barWidth));
					if (barOffset + static_cast<std::ptrdiff_t>(barCount) < 0)
					{
						// This point is completely out of the left edge, skip it
						//
						prevState = state;
						continue;
					}

					// Get y of the prevstate;
					//
					double prevStateValue = TrendScale::valueToScaleValue(prevState->value, drawParam.scaleType(), &ok);
					double prevStateY = 0;
					if (ok == true)
					{
						prevStateY = TrendScale::valueToScaledPixel(prevStateValue, signalRect, lowLimit, highLimit);
					}

					for (std::size_t cnt = 0; ok == true && cnt < barCount; cnt++)
					{
						std::ptrdiff_t index = barOffset + static_cast<std::ptrdiff_t>(cnt);
						if (index < 0)
						{
							continue;
						}

						if (static_cast<size_t>(index) >= totalBars)
						{
							// Early exit if we are out of the right edge, no need to process more bars
							//
							break;
						}

						bars[index].update(prevStateY);
					}

					// Add a bar at the last point with the value of the current state
					//
					if (size_t lastBarIndex = barOffset + barCount - 1; //
						lastBarIndex < bars.size())
					{
						double curStateValue = TrendScale::valueToScaleValue(state.value, drawParam.scaleType(), &ok);
						if (ok == true)
						{
							double curStateY = TrendScale::valueToScaledPixel(curStateValue, signalRect, lowLimit, highLimit);
							bars[lastBarIndex].update(curStateY);
						}
					}

					// --
					//
					prevState = state;
				} // for (const TrendStateItem& state : record.states)
			} // for (const TrendStateRecord& record : data)

#ifdef TREND_ZERO_COPY_TREND_DATA
#else
			hour.reset(); // Free memory, we don't need it anymore
#endif
		}

		// Draw bars
		//
		{
			painter->setPen(Qt::NoPen);
			// painter->setRenderHint(QPainter::Antialiasing, false);

			const double dpiX = drawParam.realDpiX();
			const double dpiY = drawParam.realDpiY();
			const double cosmeticPenWidth = drawParam.cosmeticPenWidth() ? drawParam.cosmeticPenWidth() : (1.0 / dpiX);
			const double cosmeticPenHeight = drawParam.cosmeticPenWidth() ? drawParam.cosmeticPenWidth() : (1.0 / dpiY);
			const double signalLineWeight = (signal.lineWeight() == 0.0) ? 1 : signal.lineWeight();
			const double minBarWidth = signalLineWeight * cosmeticPenWidth;
			const double minBarHeight = signalLineWeight * cosmeticPenHeight;
			const double pixelWidth = 1.0 / dpiX;
			auto color = signal.color();

			for (size_t i = 0; i < bars.size(); i++)
			{
				const Bar& bar = bars[i];

				if (bar.hasValue == true)
				{
					double x = signalRect.left() + i * barWidth + pixelWidth - minBarWidth / 2.0;
					double y = bar.min - minBarHeight / 2.0; // bar.min is the top of the bar, bar.max is the bottom of the bar,
															 // because in pixel coordinates, y increases downwards
					double w = minBarWidth;
					double h = std::max(bar.max - bar.min, minBarHeight);
					if (h > w)
					{
						h += minBarHeight;
					}

					painter->fillRect(QRectF{x, y, w, h}, color);
				}
			}

			// painter->setRenderHint(QPainter::Antialiasing, true);
		}

		// Reset clipping
		//
		painter->setClipping(false);
		painter->setRenderHint(QPainter::Antialiasing, true);
		return;
	}

	void TrendImpl::drawRulers(QPainter* painter, TrendParam drawParam) const
	{
		if (painter == nullptr)
		{
			Q_ASSERT(painter);
			return;
		}

		drawParam.setDpi(painter->device()->logicalDpiX(), painter->device()->logicalDpiY(), painter->device()->devicePixelRatioF());

		adjustPainter(painter, drawParam);

		// --
		//
		double dpiX = painter->device()->logicalDpiX();
		double dpiY = painter->device()->logicalDpiY();

		E::TimeType timeType = drawParam.timeType();

		std::vector<TrendSignalParam> discretes = signalSet().discreteSignals();
		std::vector<TrendSignalParam> analogs = signalSet().analogSignals();

		// Prepare drawing resources
		//
		QPen rulerPen(QBrush(qRgb(0x00, 0x00, 0xC0)), drawParam.cosmeticPenWidth(), Qt::PenStyle::DashLine);
		QPen distancePen(QBrush(qRgb(0x00, 0x00, 0xC0)), drawParam.cosmeticPenWidth(), Qt::PenStyle::SolidLine);

		QBrush backgroundBrush(drawParam.backColor1st());
		painter->setBrush(backgroundBrush);

		// --
		//
		int selectedRulerIndex = drawParam.hightlightRulerIndex();
		TimeStamp selectedRulerTime;

		if (selectedRulerIndex >= 0 && selectedRulerIndex < std::ssize(rulerSet().rulers()))
		{
			selectedRulerTime = rulerSet().rulers()[selectedRulerIndex].timeStamp();
		}

		for (int laneIndex = 0; laneIndex < drawParam.laneCount(); laneIndex++)
		{
			TimeStamp startLaneTime = TimeStamp{drawParam.startTimeStamp().timeStamp + laneIndex * drawParam.duration()};
			TimeStamp finishLaneTime = TimeStamp{startLaneTime.timeStamp + drawParam.duration()};

			TrendParam laneDrawParam = drawParam;
			laneDrawParam.setStartTimeStamp(startLaneTime);

			QRectF laneRect = calcLaneRect(laneIndex, laneDrawParam);
			QRectF trendAreaRect = calcTrendArea(laneRect, laneDrawParam);

			calcSignalRects(trendAreaRect, drawParam, &discretes, &analogs);

			std::vector<TrendRuler> laneRulers = rulerSet().rulers();
			std::sort(laneRulers.begin(),
					  laneRulers.end(),
					  [](const TrendRuler& r1, const TrendRuler& r2)
					  {
						  return r1.timeStamp() < r2.timeStamp();
					  });

			// Calc ruler timestamp text width
			//
			const auto timeStampBoundSize = calcTextSize(painter, " " + DateTimeFormat::time(true /*sec*/, true /*ms*/) + " ", drawParam);

			double rulerTextTop = laneRect.top() + (trendAreaRect.top() - laneRect.top()) / 2.0 - timeStampBoundSize.height() / 2.0;
			double rulerTextHeight = timeStampBoundSize.height();

			// Draw ruler line
			//
			painter->setClipRect(laneRect);

			double k = static_cast<double>(trendAreaRect.width()) / static_cast<double>(drawParam.duration()); // K is a coefficient

			for (size_t i = 0; i < laneRulers.size(); i++)
			{
				const TrendRuler& ruler = laneRulers[i];

				if (ruler.timeStamp() < startLaneTime)
				{
					continue;
				}

				double x = trendAreaRect.left() + k * static_cast<double>(ruler.timeStamp().timeStamp - startLaneTime.timeStamp);
				x = static_cast<double>(static_cast<int>(x * dpiX)) / dpiX; // Adjust x to look nice (not blurred)

				if (ruler.timeStamp() <= finishLaneTime)
				{
					painter->setPen(rulerPen);

					painter->drawLine(QPointF(x, trendAreaRect.top()), QPointF(x, trendAreaRect.bottom()));

					if (ruler.timeStamp() == selectedRulerTime)
					{
						double xx = static_cast<double>(x * dpiX + 1) / dpiX;

						painter->drawLine(QPointF(xx, trendAreaRect.top()), QPointF(xx, trendAreaRect.bottom()));
					}

					// Draw ruler timestamp
					//
					QString text = " " + DateTimeToString::timeMs(ruler.timeStamp().toTime()) + " ";
					QRectF textRect(x - timeStampBoundSize.width() / 2.0, rulerTextTop, timeStampBoundSize.width(), rulerTextHeight);

					painter->fillRect(textRect, backgroundBrush);
					drawText(painter, text, textRect, drawParam, Qt::AlignCenter);
				}

				// Draw distance between rulers
				//
				if (i > 0)
				{
					// There is a previous ruler, draw distance to it
					//
					const TrendRuler& prevRuler = laneRulers[i - 1];

					double prevRulerX =
						trendAreaRect.left() + k * static_cast<double>(prevRuler.timeStamp().timeStamp - startLaneTime.timeStamp);
					if (prevRulerX < trendAreaRect.left())
					{
						prevRulerX = trendAreaRect.left();
					}
					else
					{
						prevRulerX += timeStampBoundSize.width() / 2.0;
					}

					double xx = static_cast<double>(static_cast<int>(x * dpiX)) / dpiX; // Adjust x to look nice (not blurred)
					if (ruler.timeStamp() > finishLaneTime)
					{
						xx = trendAreaRect.right();
					}
					else
					{
						xx -= timeStampBoundSize.width() / 2.0;
					}

					double y = laneRect.top() + (trendAreaRect.top() - laneRect.top()) / 2.0;
					y = static_cast<double>(static_cast<int>(y * dpiY)) / dpiY; // Adjust x to look nice (not blurred)

					if (prevRulerX < xx)
					{
						painter->setPen(distancePen);
						painter->drawLine(QPointF(prevRulerX, y), QPointF(xx, y));
					}

					// Draw distance between rulers
					//
					qint64 rulersDistance = ruler.timeStamp().timeStamp - prevRuler.timeStamp().timeStamp;

					QString distanceText = QString(" " + DateTimeToString::dateTimeDurationMs(rulersDistance) + " ");

					QSizeF distanceTextBoundSize = calcTextSize(painter, distanceText, drawParam);

					if (distanceTextBoundSize.width() + distanceTextBoundSize.height() / 2 < xx - prevRulerX)
					{
						QRectF distanceTextRect;
						distanceTextRect.setLeft(prevRulerX + (xx - prevRulerX) / 2.0 - distanceTextBoundSize.width() / 2.0);
						distanceTextRect.setTop(rulerTextTop);
						distanceTextRect.setWidth(distanceTextBoundSize.width());
						distanceTextRect.setHeight(distanceTextBoundSize.height());

						painter->fillRect(distanceTextRect, backgroundBrush);
						drawText(painter, distanceText, distanceTextRect, drawParam, Qt::AlignCenter); // Draw distance between rulers
					}
				}

				if (ruler.timeStamp() > finishLaneTime)
				{
					// Break here, not in the beginning of the loop
					// We need to draw this (of lane) ruler, to draw distance to the perv ruler
					//
					break;
				}

				// Draw signals values on the ruler
				//
				if (ruler.timeStamp() >= startLaneTime && ruler.timeStamp() <= finishLaneTime)
				{
					QColor semitransparentColor = drawParam.backColor2nd();
					semitransparentColor.setAlpha(200);

					QBrush fillRectBrush(semitransparentColor);

					// Join two vectors discretes + analogs
					// x: calculated pos for ruler
					//
					std::vector<TrendSignalParam> allSignals;
					allSignals.reserve(discretes.size() + analogs.size());
					allSignals.insert(allSignals.end(), discretes.begin(), discretes.end());
					allSignals.insert(allSignals.end(), analogs.begin(), analogs.end());

					for (const TrendSignalParam& trendSignal : allSignals)
					{
						QRectF signalRect = trendSignal.tempDrawRect();

						if (signalRect.height() <= std::numeric_limits<double>::min())
						{
							continue;
						}

						TrendStateItem state = rulerSignalState(ruler, trendSignal, timeType);

						bool ok = false;

						double highLimit = TrendScale::scaleHighLimit(trendSignal, drawParam.scaleType(), &ok);
						if (ok == false)
						{
							continue;
						}

						double lowLimit = TrendScale::scaleLowLimit(trendSignal, drawParam.scaleType(), &ok);
						if (ok == false)
						{
							continue;
						}

						if (std::fabs(highLimit - lowLimit) <= std::numeric_limits<double>::min())
						{
							continue;
						}

						double value = TrendScale::valueToScaleValue(state.value, drawParam.scaleType(), &ok);
						if (ok == false)
						{
							continue;
						}

						QString str;
						if (state.isValid() == false)
						{
							// State is not available
							//
							str = "?";
						}
						else
						{
							str = TrendScale::scaleValueText(state.value, drawParam.scaleType(), trendSignal);

							// Add flags text
							//
							if (state.isValidFlag() == false)
							{
								str += " NV";
							}

							if (state.isSimulatedFlag() == true)
							{
								str += " SIM";
							}

							if (state.isBlockedFlag() == true)
							{
								str += " BL";
							}

							if (state.isMismatchFlag() == true)
							{
								str += " MIS";
							}
						}

						double vertCoef = (highLimit - lowLimit) / signalRect.height();
						double y = (value - lowLimit) / vertCoef;

						// Get text bounding rect
						//
						auto boundRect = calcTextSize(painter, str, drawParam);

						// Calc pos and draw filled rect and text
						//
						QRectF drawRect;
						if (state.isValid() == false)
						{
							// Draw in the middle of signal rect
							//
							drawRect.setLeft(x + 2.0 / dpiX);
							drawRect.setTop(signalRect.top() + signalRect.height() / 2.0 - boundRect.height() / 2.0);

							drawRect.setHeight(boundRect.height() * 1.1);
							drawRect.setWidth(boundRect.width() * 2);
						}
						else
						{
							if (trendSignal.isAnalog() == true)
							{
								drawRect.setLeft(x + 2.0 / dpiX);
								drawRect.setTop(signalRect.bottom() - y /*- boundRect.height() / 2.0*/); // just below the trend line

								drawRect.setHeight(boundRect.height() * 1.1);
								drawRect.setWidth(boundRect.width() * 1.2);

								if (drawRect.top() < signalRect.top())
								{
									drawRect.moveTop(signalRect.top());
								}

								if (drawRect.bottom() > signalRect.bottom())
								{
									drawRect.setTop(signalRect.bottom() - drawRect.height());
								}
							}
							else
							{
								drawRect.setLeft(x + 2.0 / dpiX);

								if (value <= std::numeric_limits<double>::min())
								{
									// 0
									//
									drawRect.setTop(signalRect.bottom() - boundRect.height() * 1.6);
								}
								else
								{
									// 1
									//
									drawRect.setTop(signalRect.top());
								}

								drawRect.setHeight(boundRect.height());
								drawRect.setWidth(boundRect.width() * 1.2);
							}
						}

						drawRect = signalRect.intersected(drawRect);

						if (drawRect.isEmpty() == false)
						{
							painter->fillRect(drawRect, fillRectBrush);

							painter->setPen(trendSignal.color());
							drawText(painter, str, drawRect, drawParam, Qt::AlignCenter);
						}
					} // End of draw signal values on the ruler
				}
			} // for (size_t i = 0; i < laneRulers.size(); i++)
		}

		// Reset clipping
		//
		painter->setClipping(false);

		return;
	}

	TrendStateItem TrendImpl::rulerSignalState(const TrendRuler& ruler, const TrendSignalParam& signal, E::TimeType timeType) const
	{
		const TimeStamp& rulerTime = ruler.timeStamp();

		// Getting data without requesting if it is not present
		//
		std::list<std::shared_ptr<const OneHourData>> signalData;

		TimeStamp minus1hour(rulerTime.timeStamp - 1_hour);
		TimeStamp plus1hour(rulerTime.timeStamp + 1_hour);

		signalSet().getExistingTrendData(signal, minus1hour.toDateTime(), plus1hour.toDateTime(), timeType, &signalData);

		// Look for state at point ruler.timeStamp
		//
		TrendStateItem lastState;
		lastState.clear();

		for (const std::shared_ptr<const OneHourData>& h : signalData)
		{
#ifdef TREND_ZERO_COPY_TREND_DATA
			std::shared_lock lock{h->mutex};
#endif
			const std::vector<TrendStateRecord>& records = h->data_;

			for (const TrendStateRecord& record : records)
			{
				const std::vector<TrendStateItem>& states = record.states;

				if (states.empty() == true)
				{
					continue;
				}

				if (states.back().getTime(timeType) < rulerTime)
				{
					lastState = states.back();
					continue; // to next record
				}

				//				//for (const TrendStateItem& state : states)							// for by index is faster
				//				size_t stateCount = states.size();
				//				for (size_t stateIndex = 0; stateIndex < stateCount; ++stateIndex)
				//				{
				//					const TrendStateItem& state = states[stateIndex];

				//					TimeStamp ts = state.getTime(timeType);
				//					if (ts >= rulerTime)
				//					{
				//						// Got it, we need to return prev point.
				//						// if current state not valid, then we assume last state is not valid also
				//						//
				//						if (state.isValid() == false)
				//						{
				//							return state;
				//						}
				//						else
				//						{
				//							return lastState;
				//						}
				//					}

				//					//lastState = state;
				//					static_assert(std::is_pod<TrendStateItem>::value, "TrendStateItem must be a POD type.");
				//					memcpy(&lastState, &state, sizeof(lastState));
				//				}

				static const TrendStateItem fakeState = TrendStateItem();
				auto stateIt = std::lower_bound(states.begin(),
												states.end(),
												fakeState,
												[&rulerTime, &timeType](const TrendStateItem& state, const TrendStateItem& /*fakeState*/)
												{
													return state.getTime(timeType) < rulerTime;
												});

				if (stateIt != states.end())
				{
					if (rulerTime == (*stateIt).getTime(timeType))
					{
						// That is the exact value
						//
						return *stateIt;
					}
					else
					{
						if (stateIt == states.begin()) // and it is not equal to the first (se the prev cond)
						{
							// Take the last value from the previous states vector
							//
							return lastState;
						}
						else
						{
							--stateIt;
							return *stateIt;
						}
					}
				}
				else
				{
					// value is beyond the last item
					//
					Q_ASSERT(false); // we have checked it before
				}

				Q_ASSERT(false);
			}
		}

		TrendStateItem nonValid;
		nonValid.clear();
		return nonValid;
	}

	void TrendImpl::adjustPainter(QPainter* painter, const TrendParam& trendParam)
	{
		Q_ASSERT(painter);

		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->setRenderHint(QPainter::TextAntialiasing, true);

		painter->resetTransform();

		painter->translate(0.5, 0.5);
		painter->scale(trendParam.realDpiX() / trendParam.devicePixelRatio(), trendParam.realDpiY() / trendParam.devicePixelRatio());

		return;
	}

	void TrendImpl::drawPolyline(QPainter* painter, const std::vector<QPointF>& lines, const QRectF& rect) const
	{
		Q_ASSERT(painter);

		if (lines.size() < 2)
		{
			return;
		}

		double left = rect.left();
		double right = rect.right();

		if (lines.front().x() > right || lines.back().x() < left)
		{
			return;
		}

		qsizetype size = lines.size();
		const QPointF* ptrToFirst = lines.data();

		qsizetype index = 0;
		for (; index < size; index++)
		{
			if (ptrToFirst->x() < left && index < size - 1 && (ptrToFirst + 1)->x() < left)
			{
				ptrToFirst++;
			}
			else
			{
				break;
			}
		}

		const QPointF* ptrToLast = ptrToFirst;

		for (; index < size; index++)
		{
			if (ptrToFirst->x() <= right)
			{
				ptrToLast++;
			}
			else
			{
				ptrToLast++;
				break;
			}
		}

		int d = static_cast<int>(std::distance(ptrToFirst, ptrToLast));

		painter->drawPolyline(ptrToFirst, d);

		return;
	}

	void TrendImpl::calcSignalRects(const QRectF& insideRect,
									const TrendParam& drawParam,
									std::vector<TrendSignalParam>* discretes,
									std::vector<TrendSignalParam>* analogs)
	{
		Q_ASSERT(discretes);
		Q_ASSERT(analogs);

		double y = insideRect.top();

		for (TrendSignalParam& ts : *discretes)
		{
			QRectF signalRect = {insideRect.left(), y, insideRect.width(), discreteSignalHeight};
			y += discreteSignalHeight;

			if (signalRect.top() >= insideRect.bottom())
			{
				signalRect = QRectF(); // Null rect
			}

			if (signalRect.bottom() > insideRect.bottom())
			{
				signalRect.setBottom(insideRect.bottom());
			}

			// Save calculated rect to TrendSignal
			//
			ts.setTempDrawRect(signalRect);
		}

		if (drawParam.viewMode() == E::TrendViewMode::Separated && analogs->empty() == false)
		{
			const double analogSignalsHeight = qMax((insideRect.bottom() - y) / analogs->size(), discreteSignalHeight);

			for (TrendSignalParam& ts : *analogs)
			{
				QRectF signalRect = {insideRect.left(), y, insideRect.width(), analogSignalsHeight};
				y += analogSignalsHeight;

				if (signalRect.top() >= insideRect.bottom())
				{
					signalRect = QRectF(); // Null rect
				}

				if (signalRect.bottom() > insideRect.bottom())
				{
					signalRect.setBottom(insideRect.bottom());
				}

				// Save calculated rect to TrendSignal
				//
				ts.setTempDrawRect(signalRect);
			}
		}

		if (drawParam.viewMode() == E::TrendViewMode::Overlapped && analogs->empty() == false)
		{
			const double analogSignalsHeight = qMax(insideRect.bottom() - y, discreteSignalHeight);
			QRectF signalRect = {insideRect.left(), y, insideRect.width(), analogSignalsHeight};

			if (signalRect.top() >= insideRect.bottom())
			{
				signalRect = QRectF(); // Null rect
			}

			if (signalRect.bottom() > insideRect.bottom())
			{
				signalRect.setBottom(insideRect.bottom());
			}

			for (TrendSignalParam& ts : *analogs)
			{
				// Save calculated rect to TrendSignal
				//
				ts.setTempDrawRect(signalRect);
			}
		}

		Q_ASSERT(drawParam.viewMode() == E::TrendViewMode::Separated || drawParam.viewMode() == E::TrendViewMode::Overlapped);

		return;
	}

	QRectF TrendImpl::calcProjectNameRect(const TrendParam& drawParam)
	{
		QRectF result;

		if (drawParam.project().isEmpty() == true)
		{
			return result;
		}

		result = drawParam.rectIn();

		result.setTop(result.top());
		result.setLeft(result.left() + 1.0 / 32.0);

		result.setWidth(result.width() - (1.0 / 32.0));
		result.setHeight(1.0 / 6.0);

		return result;
	}

	QRectF TrendImpl::calcLaneRect(size_t laneIndex, const TrendParam& drawParam)
	{
		const QRectF& drawRectInch = drawParam.rectIn();
		double projectNameHeight = calcProjectNameRect(drawParam).height();

		double laneMargin = 1.0 / 32.0; // 1/16 inch
		double laneHeight =
			(drawRectInch.height() - laneMargin - projectNameHeight) / static_cast<double>(drawParam.laneCount()) - laneMargin;

		QRectF laneRect;

		laneRect.setLeft(drawRectInch.left() + laneMargin);
		laneRect.setWidth(drawRectInch.width() - laneMargin * 2.0);

		laneRect.setTop(drawRectInch.top() + projectNameHeight + laneMargin + static_cast<double>(laneIndex) * (laneHeight + laneMargin));
		laneRect.setHeight(laneHeight);

		return laneRect;
	}

	QRectF TrendImpl::calcTrendArea(const QRectF& laneRect, const TrendParam& drawParam) const
	{
		return TrendImpl::calcTrendArea(laneRect, drawParam, std::span<const TrendSignalParam>{signalSet().analogSignals()});
	}

	QRectF TrendImpl::calcTrendArea(const QRectF& laneRect,
									const TrendParam& drawParam,
									std::span<const TrendSignalParam> analogSignalParams)
	{
		// Calc InsideRect(trendArea)
		// +--------------------------------+
		// |   +---------------------------+|
		// |   |   insideRect (trendArea)  ||
		// |   +---------------------------+|
		// |                                |
		// +--------------------------------+
		//
		double topMargin = 1.0 / 4.0; // 1/4 inch

		double bottomMargin = 0;
		if (drawParam.showTimeLabels() == true && drawParam.showDateLabels() == true)
		{
			bottomMargin = 1.0 / 4.0 + 1.0 / 8.0;
		}
		else
		{
			bottomMargin = 1.0 / 4.0;
		}


		QRectF insideRect;

		if (drawParam.showSignalScales() == true)
		{
			int columnCount = calcAnalogScaleColumnCount(analogSignalParams, drawParam);
			if (columnCount == 1)
			{
				// 1 column of scale values
				//
				insideRect.setLeft(laneRect.left() + 3.0 / 4.0);
				insideRect.setRight(laneRect.right() - 1.0 / 4.0);
				insideRect.setTop(laneRect.top() + topMargin);
				insideRect.setBottom(laneRect.bottom() - bottomMargin);
			}
			else
			{
				// 2 columns of scale values
				//
				insideRect.setLeft(laneRect.left() + 3.0 / 4.0 * 1.5);
				insideRect.setRight(laneRect.right() - 1.0 / 4.0);
				insideRect.setTop(laneRect.top() + topMargin);
				insideRect.setBottom(laneRect.bottom() - bottomMargin);
			}
		}
		else
		{
			insideRect.setLeft(laneRect.left() + 1.0 / 4.0);
			insideRect.setRight(laneRect.right() - 1.0 / 4.0);
			insideRect.setTop(laneRect.top() + topMargin);
			insideRect.setBottom(laneRect.bottom() - bottomMargin);
		}

		// Adjust inside rect to dpiX, so it will look pretty while drawing it with cosmetic pen
		//
		double dpiX = drawParam.realDpiX();
		double dpiY = drawParam.realDpiY();

		insideRect.setLeft(static_cast<double>(static_cast<int>(insideRect.left() * dpiX)) / dpiX);
		insideRect.setTop(static_cast<double>(static_cast<int>(insideRect.top() * dpiY)) / dpiY);
		insideRect.setWidth(static_cast<double>(static_cast<int>(insideRect.width() * dpiX)) / dpiX);
		insideRect.setHeight(static_cast<double>(static_cast<int>(insideRect.height() * dpiY)) / dpiY);

		return insideRect;
	}

	int TrendImpl::calcAnalogScaleColumnCount(std::span<const TrendSignalParam> analogSignalParams, const TrendParam& drawParam)
	{
		if (drawParam.viewMode() == E::TrendViewMode::Separated)
		{
			return 1;
		}

		if (drawParam.viewMode() == E::TrendViewMode::Overlapped)
		{
			const auto& analogs = analogSignalParams;
			if (analogs.empty() == true)
			{
				return 1;
			}

			auto sameSignalLimitsPred = [scaleType = drawParam.scaleType(), &firstSignal = analogs[0]](const TrendSignalParam& sp)
			{
				return sp.viewLowLimit(scaleType) == firstSignal.viewLowLimit(scaleType) &&   // Same low limit
					   sp.viewHighLimit(scaleType) == firstSignal.viewHighLimit(scaleType) && // Same high limit
					   sp.analogFormat() == firstSignal.analogFormat() &&                     // Same format
					   sp.precision() == firstSignal.precision();                             // Same precision
			};

			if (analogs.size() <= 3 || std::ranges::all_of(analogs, sameSignalLimitsPred) == true)
			{
				return 1;
			}

			return 2;
		}

		// New mode?
		//
		Q_ASSERT(false);
		return 1;
	}

	QRectF TrendImpl::calcScaleAreaRect(const QRectF& laneRect, const QRectF& signalRect)
	{
		// +------------laneRect------------+
		// | R +---------------------------+|
		// | e |                           ||
		// | s |        SignalRect         ||
		// | u |                           ||
		// | l |                           ||
		// | t +---------------------------+|
		// | . |                           ||
		// | . |                           ||
		// | . +---------------------------+|
		// +--------------------------------+
		//
		QRectF result(QPointF(laneRect.left(), signalRect.top()), QPointF(signalRect.left(), signalRect.bottom()));

		return result;
	}

	QRect TrendImpl::inchRectToPixelRect(const QRectF& rect, const TrendParam& drawParam)
	{
		QRect result(static_cast<int>(rect.left() * drawParam.dpiX()),
					 static_cast<int>(rect.top() * drawParam.dpiY()),
					 static_cast<int>(rect.width() * drawParam.dpiX()),
					 static_cast<int>(rect.height() * drawParam.dpiY()));

		return result;
	}

	QRectF TrendImpl::pixelRectToInchRect(const QRect& rect, const TrendParam& drawParam)
	{
		QRectF result(static_cast<double>(rect.left()) / static_cast<double>(drawParam.dpiX()),
					  static_cast<double>(rect.top()) / static_cast<double>(drawParam.dpiY()),
					  static_cast<double>(rect.width()) / static_cast<double>(drawParam.dpiX()),
					  static_cast<double>(rect.height()) / static_cast<double>(drawParam.dpiY()));

		return result;
	}

	QPoint TrendImpl::inchPointToPixelPoint(const QPointF& point, const TrendParam& drawParam)
	{
		QPoint result(static_cast<int>(point.x() * drawParam.dpiX()), static_cast<int>(point.y() * drawParam.dpiY()));
		return result;
	}

	QPointF TrendImpl::pixelPointToInchPoint(const QPoint& point, const TrendParam& drawParam)
	{
		QPointF result(static_cast<double>(point.x()) / static_cast<double>(drawParam.dpiX()),
					   static_cast<double>(point.y()) / static_cast<double>(drawParam.dpiY()));
		return result;
	}

	TrendImpl::MouseOn TrendImpl::mouseIsOver(QPoint mousePos,
											  const TrendParam& drawParam,
											  int* outLaneIndex,
											  TimeStamp* outTime,
											  int* rulerIndex,
											  TrendSignalParam* outSignal) const
	{
		if (outLaneIndex == nullptr || outTime == nullptr || rulerIndex == nullptr || outSignal == nullptr)
		{
			Q_ASSERT(outLaneIndex);
			Q_ASSERT(outTime);
			Q_ASSERT(rulerIndex);
			Q_ASSERT(outSignal);
			return TrendImpl::MouseOn::Outside;
		}

		*outLaneIndex = -1;
		*rulerIndex = -1;
		*outTime = TimeStamp();

		// Transform mousePos to inches, as everything for drawing is done in inches
		//
		QPointF pos(static_cast<double>(mousePos.x()) / drawParam.dpiX(), static_cast<double>(mousePos.y()) / drawParam.dpiY());

		// MouseOn::Outside
		//
		if (drawParam.rectPx().contains(mousePos) == false)
		{
			return MouseOn::Outside;
		}

		// --
		//
		for (int laneIndex = 0; laneIndex < drawParam.laneCount(); laneIndex++)
		{
			QRectF laneRect = calcLaneRect(laneIndex, drawParam);

			if (laneRect.contains(pos) == true)
			{
				QRectF trendArea = calcTrendArea(laneRect, drawParam);

				if (trendArea.contains(pos) == true)
				{
					// Calc time
					//
					qint64 startLaneTime = drawParam.startTimeStamp().timeStamp + laneIndex * drawParam.duration();
					double coef = drawParam.duration() / trendArea.width();
					qint64 timeOffset = static_cast<qint64>((pos.x() - trendArea.left()) * coef);
					TimeStamp posTime = TimeStamp{startLaneTime + timeOffset};

					*outTime = posTime;
					*outLaneIndex = laneIndex;

					auto discretes = signalSet().discreteSignals();
					auto analogs = signalSet().analogSignals();

					calcSignalRects(trendArea, drawParam, &discretes, &analogs);

					for (const TrendSignalParam& tsp : discretes)
					{
						if (tsp.tempDrawRect().contains(pos) == true)
						{
							*outSignal = tsp;
							break;
						}
					}

					for (const TrendSignalParam& tsp : analogs)
					{
						if (tsp.tempDrawRect().contains(pos) == true)
						{
							*outSignal = tsp;
							break;
						}
					}

					// Check if pos OnRuler
					//
					qint64 deltaTime = static_cast<qint64>(coef * 1.0 / 32.0);
					const std::vector<TrendRuler>& rulers = rulerSet().rulers();
					int ri = 0;

					for (const TrendRuler& ruler : rulers)
					{
						if (posTime >= TimeStamp(ruler.timeStamp().timeStamp - deltaTime) &&
							posTime <= TimeStamp(ruler.timeStamp().timeStamp + deltaTime))
						{
							*rulerIndex = ri;
							return MouseOn::OnRuler;
						}

						ri++;
					}

					// Check if pos on signal description
					//
					for (const auto& [signalId, descriptionRect] : drawParam.signalDescriptionRect())
					{
						if (descriptionRect.contains(pos) == true)
						{
							for (const TrendSignalParam& tsp : discretes)
							{
								if (tsp.appSignalId() == signalId)
								{
									*outSignal = tsp;
									break;
								}
							}

							for (const TrendSignalParam& tsp : analogs)
							{
								if (tsp.appSignalId() == signalId)
								{
									*outSignal = tsp;
									break;
								}
							}

							return MouseOn::OnSignalDescription;
						}
					}

					// --
					//
					return MouseOn::InsideTrendArea;
				}
				else
				{
					*outLaneIndex = laneIndex;
					return MouseOn::OutsideTrendArea;
				}
			}
		}

		// --
		//
		*outLaneIndex = -1;
		return MouseOn::Outside; // Can be frame between lanes
	}

	void TrendImpl::drawText(QPainter* painter,
							 const QString& str,
							 const QRectF& rect,
							 const TrendParam& drawParam,
							 int flags,
							 QRectF* boundingRect /* = nullptr*/)
	{
		if (painter == nullptr)
		{
			Q_ASSERT(painter);
			return;
		}

		painter->save();

		auto worldTransform = painter->worldTransform();
		QTransform scaleTransform{1.0 / drawParam.devicePixelRatio(),
								  worldTransform.m12(),
								  worldTransform.m21(),
								  1.0 / drawParam.devicePixelRatio(),
								  worldTransform.dx(),
								  worldTransform.dy()};
		painter->setWorldTransform(scaleTransform);

		// Set font
		//
		double realDpiX = drawParam.realDpiX();
		double realDpiY = drawParam.realDpiY();

		QFont font;

		int pixelSize = static_cast<int>(textSizeMm / 25.4 * realDpiY);
		font.setPixelSize(pixelSize);

		painter->setFont(font);

		// --
		//
		QRectF rc{rect.left() * realDpiX, rect.top() * realDpiY, rect.width() * realDpiX, rect.height() * realDpiY};

		QRectF boundingRectIn;

		painter->drawText(rc, flags, str, &boundingRectIn);

		if (boundingRect != nullptr)
		{
			*boundingRect = QRectF{boundingRectIn.left() / realDpiX,
								   boundingRectIn.top() / realDpiY,
								   boundingRectIn.width() / realDpiX,
								   boundingRectIn.height() / realDpiY};
		}

		painter->restore();
		return;
	}

	QSizeF TrendImpl::calcTextSize(QPainter* painter, const QString& str, const TrendParam& drawParam)
	{
		painter->save();
		auto worldTransform = painter->worldTransform();
		QTransform scaleTransform{1.0 / drawParam.devicePixelRatio(),
								  worldTransform.m12(),
								  worldTransform.m21(),
								  1.0 / drawParam.devicePixelRatio(),
								  worldTransform.dx(),
								  worldTransform.dy()};
		painter->setWorldTransform(scaleTransform);

		// Set font
		//
		double realDpiY = drawParam.realDpiY();

		QFont font;

		int pixelSize = static_cast<int>(textSizeMm / 25.4 * realDpiY);
		font.setPixelSize(pixelSize);
		painter->setFont(font);

		QRectF boundingRect;
		boundingRect = painter->boundingRect(QRectF{}, 0, str);

		QSizeF result{boundingRect.width() / drawParam.realDpiX(), boundingRect.height() / drawParam.realDpiY()};

		painter->restore();
		return result;
	}

	QUuid TrendImpl::uuid() const
	{
		return m_uuid;
	}

	void TrendImpl::setUuid(QUuid value)
	{
		m_uuid = value;
	}

	TrendLib::TrendSignalSet& TrendImpl::signalSet()
	{
		return m_signalSet;
	}

	const TrendLib::TrendSignalSet& TrendImpl::signalSet() const
	{
		return m_signalSet;
	}

	TrendLib::TrendRulerSet& TrendImpl::rulerSet()
	{
		return m_rulerSet;
	}

	const TrendLib::TrendRulerSet& TrendImpl::rulerSet() const
	{
		return m_rulerSet;
	}
} // namespace TrendLib