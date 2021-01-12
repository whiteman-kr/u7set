#include "Trend.h"
#include "../lib/CUtils.h"
#include "../Proto/trends.pb.h"
#include "TrendScale.h"

namespace TrendLib
{

	const double Trend::discreteSignalHeight = 5.0 / 8.0;		// if inches

	Trend::Trend()
	{
	}

	bool Trend::save(::Proto::Trend* message) const
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

	bool Trend::load(const ::Proto::Trend& message)
	{
		if (message.IsInitialized() == false)
		{
			Q_ASSERT(message.IsInitialized());
			return false;
		}

		bool ok = true;

		ok &= m_signalSet.load(message.signal_set());
		ok &= m_rulerSet.load(message.ruler_set());

		return true;
	}

	void Trend::draw(QImage* image, const TrendParam& drawParam) const
	{
		if (image == nullptr)
		{
			Q_ASSERT(image);
			return;
		}

		QTime timeMeasures;
		timeMeasures.start();

		image->fill(Qt::white);

		// --
		//
		QPainter painter(image);

		draw(&painter, drawParam, true);

		//qDebug() << "Trend draw time: " << timeMeasures.elapsed() << " ms";
		return;
	}

	void Trend::draw(QPainter* painter, const TrendParam& drawParam, bool needAdjustPainter) const
	{
		Q_ASSERT(painter);

		if (needAdjustPainter == true)
		{
			adjustPainter(painter, drawParam.dpiX(), drawParam.dpiY());
		}

		// --
		//
		QDateTime startTime = drawParam.startTime();

		for (int laneIndex = 0; laneIndex < drawParam.laneCount(); laneIndex++)
		{
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			TrendParam laneDrawParam = drawParam;
			laneDrawParam.setStartTime(startTime);

			QRectF laneRect = calcLaneRect(laneIndex, drawParam);

			drawLane(painter, laneRect, laneDrawParam);			// Draw whole lane

			// As laneDrawParam is a copy of drawParam, we need to copy from
			// laneDrawParam to drawParam vector signalDescriptionRect
			//
			drawParam.signalDescriptionRect().insert(
						drawParam.signalDescriptionRect().end(),
						laneDrawParam.signalDescriptionRect().begin(),
						laneDrawParam.signalDescriptionRect().end());

			startTime = startTime.addMSecs(laneDrawParam.duration());
		}

		return;
	}

	void Trend::drawLane(QPainter* painter, const QRectF& laneRect, const TrendParam& drawParam) const
	{
		painter->setBrush(drawParam.backColor1st());
		painter->setPen(Qt::PenStyle::NoPen);
		painter->drawRect(laneRect);

		std::vector<TrendSignalParam> discretes = signalSet().discreteSignals();
		std::vector<TrendSignalParam> analogs = signalSet().analogSignals();

		// Calc InsideRect
		// +-------------------------------+
		// |  +---------------------------+|
		// |  |   insideRect (trendArea)  ||
		// |  +---------------------------+|
		// +-------------------------------+
		//
		QRectF insideRect = calcTrendArea(laneRect, drawParam);

		// Calc signals rects, calculates rect will be written to discretes/analogs
		//
		calcSignalRects(insideRect, drawParam, &discretes, &analogs);

		// Draw backgrounds
		//
		drawBackground(painter, insideRect, drawParam, discretes, analogs);

		// Draw Time grid
		//
		drawTimeGrid(painter, laneRect, insideRect, drawParam);

		// Draw vertical scale, signal id and caption
		//
		drawSignalsDecor(painter, laneRect, drawParam, discretes, analogs);

		// Draw signal trend
		//
		for (const TrendSignalParam& signal : discretes)
		{
			drawSignalTrend(painter, signal, drawParam);
		}

		for (const TrendSignalParam& signal : analogs)
		{
			drawSignalTrend(painter, signal, drawParam);
		}

		return;
	}

	void Trend::drawBackground(QPainter* painter,
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

		for (const TrendSignalParam& ts: discretes)
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
		Q_ASSERT(drawParam.viewMode() == TrendViewMode::Separated ||
				 drawParam.viewMode() == TrendViewMode::Overlapped);

		if (analogs.empty() == true &&
			lastDiscreteRect.isEmpty() == false)
		{
			// Draw backgorund in switched color, it is just nice to separate discretes from empty area
			//
			QRectF blankArea(lastDiscreteRect.bottomLeft(),
							 insideRect.bottomRight());

			signalBackColor = (signalBackColor == drawParam.backColor1st()) ? drawParam.backColor2nd() : drawParam.backColor1st();
			painter->fillRect(blankArea, signalBackColor);
		}

		if (drawParam.viewMode() == TrendViewMode::Separated  &&
			analogs.empty() == false)
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

		if (drawParam.viewMode() == TrendViewMode::Overlapped &&
			analogs.empty() == false)
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

	void Trend::drawTimeGrid(QPainter* painter, const QRectF& laneRect, const QRectF& insideRect, const TrendParam& drawParam) const
	{
		double dpiX = drawParam.dpiX();

		// Calc time grid
		//
		static const std::array<qint64, 31> possibleTimeGridIntervals = {5_ms, 10_ms, 20_ms, 25_ms, 50_ms,
																		 100_ms, 200_ms, 250_ms, 500_ms,
																		 1_sec, 2_sec, 5_sec, 10_sec, 15_sec, 20_sec, 30_sec,
																		 1_min, 90_sec, 2_min, 5_min, 10_min, 15_min, 20_min, 30_min,
																		 1_hour, 2_hours, 3_hours, 6_hours, 12_hours, 24_hours,
																		 24_hours * 7};

		QRectF boundRect;
		QString estimatedString = (drawParam.duration() < 10_sec) ? "HH:MM:SS.XXX" : "HH:MM:SS";
		drawText(painter, estimatedString, QRectF(), drawParam, Qt::AlignCenter, &boundRect);

		double minTimeInterval = boundRect.width() * 1.4;

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
			double x;
			TimeStamp timeStamp;
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

			// Make sure that x is proper alligned for nice look of cosmetic pen
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

			if (lastDate != date &&
				time == QTime{0, 0, 0, 0})
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
		QRectF textClipRect(laneRect.left(), insideRect.bottom(), laneRect.width(), laneRect.bottom() - insideRect.bottom());
		painter->setClipRect(textClipRect);

		painter->setPen(Qt::black);
		QString lastDateText;
		for (const PosTimePair& p : timeGridPos)
		{
			QDateTime dateTime = p.timeStamp.toDateTime();
			QString timeText = CUtils::dateTimeToStringTime(dateTime, timeGridInterval < 1_sec);
			QString dateText = CUtils::dateTimeToStringDate(dateTime);

			QRectF timeTextRect(p.x - 2.0, insideRect.bottom(), 4.0, (laneRect.bottom() - insideRect.bottom()) / 2.0);
			QRectF dateTextRect(p.x - 2.0, timeTextRect.bottom(), 4.0, (laneRect.bottom() - insideRect.bottom()) / 2.0);

			drawText(painter, timeText, timeTextRect, drawParam, Qt::AlignCenter);

			if (lastDateText != dateText)
			{
				lastDateText = dateText;
				drawText(painter, dateText, dateTextRect, drawParam, Qt::AlignCenter);
			}
		}

		painter->setClipping(false);

		// --
		//

		return;
	}

	void Trend::drawSignalsDecor(QPainter* painter,
								 const QRectF& laneRect,
								 const TrendParam& drawParam,
								 const std::vector<TrendSignalParam>& discretes,
								 const std::vector<TrendSignalParam>& analogs) const
	{
		Q_ASSERT(painter);
		painter->setClipRect(laneRect);

		// Draw DISCRETE signal id, caption and scale ("0", "1")
		//
		for (const TrendSignalParam& ts: discretes)
		{
			QRectF signalRect = ts.tempDrawRect();
			if (signalRect.isNull() == true)
			{
				break;
			}

			QString signalText = QString("  %1 - %2").arg(ts.signalId()).arg(ts.caption());

			painter->setPen(ts.color());

			QRectF testDesctriptionBoundRect;
			drawText(painter, signalText, signalRect, drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, &testDesctriptionBoundRect);

			auto scr = std::make_pair(ts.appSignalId(), testDesctriptionBoundRect);
			drawParam.signalDescriptionRect().push_back(scr);

			// Draw scale 0/1 for discretes
			//
			QRectF scaleAreaRect = calcScaleAreaRect(laneRect, signalRect);

			drawText(painter, "0 ", scaleAreaRect, drawParam, Qt::AlignRight | Qt::AlignBottom);
			drawText(painter, "1 ", scaleAreaRect, drawParam, Qt::AlignRight | Qt::AlignTop);
		}

		// Draw ANALOG signal id, caption and scale for TrendView::Separated mode
		//
		Q_ASSERT(drawParam.viewMode() == TrendViewMode::Separated ||
				 drawParam.viewMode() == TrendViewMode::Overlapped);

		if (drawParam.viewMode() == TrendViewMode::Separated  &&
			analogs.empty() == false)
		{
			for (const TrendSignalParam& ts : analogs)
			{
				QRectF signalRect = ts.tempDrawRect();
				if (signalRect.isNull() == true)
				{
					break;
				}

				QString signalText;
				if (ts.unit().isEmpty() == true)
				{
					signalText = QString("  %1 - %2").arg(ts.signalId()).arg(ts.caption());
				}
				else
				{
					signalText = QString("  %1 - %2, %3").arg(ts.signalId()).arg(ts.caption()).arg(ts.unit());
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

				painter->setPen(ts.color());

				// Draw description text

				QRectF testDesctriptionBoundRect;
				drawText(painter, signalText, signalRect, drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, &testDesctriptionBoundRect);

				auto scr = std::make_pair(ts.appSignalId(), testDesctriptionBoundRect);
				drawParam.signalDescriptionRect().push_back(scr);

				// Draw horizontal grid and scale
				//
				drawAnalogSignalsGridSeparateMode(painter, laneRect, drawParam, ts);
			}
		}

		// Draw ANALOG signal id, caption and scale for TrendView::Overlapped mode
		//
		if (drawParam.viewMode() == TrendViewMode::Overlapped  &&
			analogs.empty() == false)
		{
			QRectF signalRect = analogs.front().tempDrawRect();

			for (const TrendSignalParam& ts : analogs)
			{
				QString signalText;
				if (ts.unit().isEmpty() == true)
				{
					signalText = QString("  %1 - %2").arg(ts.signalId()).arg(ts.caption());
				}
				else
				{
					signalText = QString("  %1 - %2, %3").arg(ts.signalId()).arg(ts.caption()).arg(ts.unit());
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

				painter->setPen(ts.color());

				QRectF testDesctriptionBoundRect;
				drawText(painter, signalText, signalRect, drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, &testDesctriptionBoundRect);

				auto scr = std::make_pair(ts.appSignalId(), testDesctriptionBoundRect);
				drawParam.signalDescriptionRect().push_back(scr);

				// Shift rect
				//
				signalRect.setTop(testDesctriptionBoundRect.bottom() + testDesctriptionBoundRect.height() * 0.25);
			}

			// Draw horizontal grid and scale
			//
			drawAnalogSignalsGridOverlappedMode(painter, laneRect, drawParam, analogs);
		}

		//		// --
		//		//
		painter->setClipping(false);

		return;
	}

	void Trend::drawAnalogSignalsGridSeparateMode(QPainter* painter,
												  const QRectF& laneRect,
												  const TrendParam& drawParam,
												  const TrendSignalParam& signal) const
	{
		Q_ASSERT(painter);
		painter->setClipping(false);

		QRectF signalRect = signal.tempDrawRect();
		QRectF scaleAreaRect = calcScaleAreaRect(laneRect, signalRect);

		if (signalRect.isEmpty() == true ||
			scaleAreaRect.isEmpty() == true)
		{
			return;
		}

		bool ok  = false;

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
		double minInchInterval = 1.0/4.0;	// 1/4 in -- minimum inches interval

		auto scaleValues = TrendScale::scaleValues(drawParam.scaleType(), lowLimit, highLimit, signalRect, minInchInterval); // first: value, second: display value
		if (scaleValues.has_value() == false)
		{
			return;
		}

		// Draw horz grids
		//
		double dpiY = drawParam.dpiY();

		QPen gridPen(Qt::lightGray, drawParam.cosmeticPenWidth(), Qt::PenStyle::DashLine);
		painter->setPen(gridPen);

		std::vector<std::pair<double, double>> grids;		// first: y pos, second: display value
		grids.reserve(scaleValues->size());

		for (const std::pair<double, double>& p : *scaleValues)
		{
			double value = p.first;

			double y = TrendScale::valueToScaledPixel(value, signalRect, lowLimit, highLimit);

			double antiAliasedY = static_cast<double>(static_cast<int>(y * dpiY)) / dpiY;		// Align to DPI

			if (antiAliasedY < signalRect.top() ||
				antiAliasedY > signalRect.bottom())
			{
				continue;
			}

			painter->drawLine(QPointF(signalRect.left(), antiAliasedY),
							  QPointF(signalRect.right(), antiAliasedY));

			double scaleValue = p.second;

			grids.emplace_back(y, scaleValue);
		}

		// Draw grid values
		//
		QRectF boundTextRect;
		drawText(painter, "0", QRectF(), drawParam, Qt::AlignCenter, &boundTextRect);
		double textHeight = boundTextRect.height();

		painter->setPen(signal.color());

		painter->setClipRect(scaleAreaRect);

		for (const std::pair<double, double>& p : grids)
		{
			double y = p.first;
			double value = p.second;

			QRectF textRect(scaleAreaRect.left(), y - textHeight / 2.0, scaleAreaRect.width(), textHeight);

			if (textRect.top() < scaleAreaRect.top() ||
				textRect.bottom() > scaleAreaRect.bottom())
			{
				continue;
			}

			QString text = TrendScale::scaleValueText(value, drawParam.scaleType(), signal);

			drawText(painter, text, textRect, drawParam, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextDontClip);
		}

		painter->setClipping(false);
		return;
	}

	void Trend::drawAnalogSignalsGridOverlappedMode(QPainter* painter,
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

		if (analogs.size() == 1)
		{
			drawAnalogSignalsGridSeparateMode(painter, laneRect, drawParam, analogs[0]);
			return;
		}

		QRectF signalRect = analogs[0].tempDrawRect();
		QRectF scaleAreaRect = calcScaleAreaRect(laneRect, signalRect);

		if (signalRect.isEmpty() == true ||
			scaleAreaRect.isEmpty() == true)
		{
			return;
		}

		bool ok  = false;

		double highLimit = TrendScale::scaleHighLimit(analogs[0], drawParam.scaleType(), &ok);
		if (ok == false)
		{
			return;
		}

		double lowLimit = TrendScale::scaleLowLimit(analogs[0], drawParam.scaleType(), &ok);
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
		double minInchInterval = 3.0/8.0;	// minimum inches interval

		auto scaleValues = TrendScale::scaleValues(drawParam.scaleType(), lowLimit, highLimit, signalRect, minInchInterval); // first: value, second: display value
		if (scaleValues.has_value() == false)
		{
			return;
		}

		// Draw horz grids
		//
		double dpiY = drawParam.dpiY();

		QPen gridPen(Qt::lightGray, drawParam.cosmeticPenWidth(), Qt::PenStyle::DashLine);
		painter->setPen(gridPen);

		std::vector<std::pair<double, double>> grids;		// first: y pos, second: display value
		grids.reserve(scaleValues->size());

		for (const std::pair<double, double>& p : *scaleValues)
		{
			double value = p.first;

			double y = TrendScale::valueToScaledPixel(value, signalRect, lowLimit, highLimit);

			double antiAliasedY = static_cast<double>(static_cast<int>(y * dpiY)) / dpiY;		// Align to DPI

			if (antiAliasedY < signalRect.top() ||
				antiAliasedY > signalRect.bottom())
			{
				continue;
			}

			painter->drawLine(QPointF(signalRect.left(), antiAliasedY),
							  QPointF(signalRect.right(), antiAliasedY));

			double scaleValue = p.second;

			grids.emplace_back(y, scaleValue);
		}

		// Draw grid values for the FIRST signal
		//
		painter->setPen(analogs[0].color());

		QRectF boundTextRect;
		drawText(painter, "0", QRectF(), drawParam, Qt::AlignCenter, &boundTextRect);
		double textHeight = boundTextRect.height();

		painter->setClipRect(scaleAreaRect);

		for (const std::pair<double, double>& p : grids)
		{
			double y = p.first;
			double value = p.second;

			// This signal is draw in 0 pos
			//  2 | 0
			// ---+---
			//  3 | 1
			QRectF textRect(scaleAreaRect.left() + scaleAreaRect.width() / 2.0,
							y - textHeight,
							scaleAreaRect.width() / 2.0,
							textHeight);

			if (textRect.top() < scaleAreaRect.top() ||
				textRect.bottom() > scaleAreaRect.bottom())
			{
				continue;
			}

			QString text = TrendScale::scaleValueText(value, drawParam.scaleType(), analogs[0]);

			drawText(painter, text, textRect, drawParam, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextDontClip);
		}

		// Draw grid values for the rest of signlas
		//
		for (size_t i = 1; i < analogs.size(); i++)
		{
			const TrendSignalParam& signal = analogs[i];
			Q_ASSERT(signal.isAnalog() == true);

			double signalHighLimit = TrendScale::scaleHighLimit(signal, drawParam.scaleType(), &ok);
			if (ok == false)
			{
				continue;
			}

			double signalLowLimit = TrendScale::scaleLowLimit(signal, drawParam.scaleType(), &ok);
			if (ok == false)
			{
				continue;
			}

			double signalDelta = signalHighLimit - signalLowLimit;
			if (std::fabs(signalDelta) <= std::numeric_limits<double>::min())
			{
				// Divide by 0 possible
				//
				continue;
			}

			painter->setPen(signal.color());

			for (const std::pair<double, double>& p : grids)
			{
				double y = p.first;

				double scaleValue = TrendScale::scaledPixelToValue(y, signalRect, signalLowLimit, signalHighLimit);

				double value = TrendScale::valueFromScaleValue(scaleValue, drawParam.scaleType(), &ok);

				// This signal is draw in 0 pos
				//  2 | 0
				// ---+---
				//  3 | 1
				QRectF textRect;

				switch (signal.tempSignalIndex() % 4)
				{
				case 0:
					textRect = QRectF(scaleAreaRect.left() + scaleAreaRect.width() / 2.0,
									  y - textHeight,
									  scaleAreaRect.width() / 2.0,
									  textHeight);
					break;
				case 1:
					textRect = QRectF(scaleAreaRect.left() + scaleAreaRect.width() / 2.0,
									  y,
									  scaleAreaRect.width() / 2.0,
									  textHeight);
					break;
				case 2:
					textRect = QRectF(scaleAreaRect.left(),
									  y - textHeight,
									  scaleAreaRect.width() / 2.0,
									  textHeight);
					break;
				case 3:
					textRect = QRectF(scaleAreaRect.left(),
									  y,
									  scaleAreaRect.width() / 2.0,
									  textHeight);
					break;
				}

				if (textRect.top() < scaleAreaRect.top() ||
					textRect.bottom() > scaleAreaRect.bottom())
				{
					continue;
				}

				QString text = ok == true ? TrendScale::scaleValueText(value, drawParam.scaleType(), signal) : "?";

				drawText(painter, text, textRect, drawParam, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextDontClip);
			}
		}

		painter->setClipping(false);
		return;
	}

	void Trend::drawSignalTrend(QPainter* painter, const TrendSignalParam& signal, const TrendParam& drawParam) const
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

		std::list<std::shared_ptr<OneHourData>> signalData;

		bool requestResult = signalSet().getTrendData(signal.appSignalId(), startTime, finishTime, drawParam.timeType(), &signalData);
		if (requestResult == false)
		{
			signalData.clear();
		}

		// --
		//
		if (signal.isDiscrete() == true)
		{
			drawSignalTrendDiscrete(painter, signal, drawParam, signalData);
		}

		if (signal.isAnalog() == true)
		{
			drawSignalTrendAnalog(painter, signal, drawParam, signalData);
		}

		// --
		//
		return;
	}

	void Trend::drawSignalTrendDiscrete(QPainter* painter, const TrendSignalParam& signal, const TrendParam& drawParam, const std::list<std::shared_ptr<OneHourData>>& signalData) const
	{
		Q_ASSERT(painter);
		Q_ASSERT(signal.isDiscrete() == true);

		QRectF signalRect = signal.tempDrawRect();

		// Set clip region
		//
		painter->setClipRect(signalRect);

		// Draw trend
		//
		QRectF textBoundRect;
		drawText(painter, "0", textBoundRect, drawParam, Qt::AlignLeft | Qt::AlignTop, &textBoundRect);

		E::TimeType timeType = drawParam.timeType();

		QPen linePen({signal.color()},
					 (signal.lineWeight() <= 1.0) ? drawParam.cosmeticPenWidth() : signal.lineWeight() / drawParam.dpiY(),
					 Qt::SolidLine);
		painter->setPen(linePen);

		static const int recomendedSize = 8192;
		QVector<QPointF> lines;
		lines.reserve(recomendedSize);

		TimeStamp startTimeStamp = drawParam.startTimeStamp();
		qint64 duration = drawParam.duration();

		double dpiY = drawParam.dpiY();

		double yPos0 = signalRect.bottom() - textBoundRect.height() / 2.0;
		double yPos1 = signalRect.top() + textBoundRect.height() * 1.1;
		yPos0 = static_cast<double>(static_cast<int>(yPos0 * dpiY)) / dpiY;		// Make sure that Y is proper alligned for nice look of cosmetic pen
		yPos1 = static_cast<double>(static_cast<int>(yPos1 * dpiY)) / dpiY;		// Make sure that Y is proper alligned for nice look of cosmetic pen

		double rectRight = signalRect.right();

		double lastX = 0;
		double lastY = 0;

		//int pointIndex = 0;

		for (std::shared_ptr<OneHourData> hour : signalData)
		{
			const std::vector<TrendStateRecord>& data = hour->data;

			for (const TrendStateRecord& record : data)
			{
				for (const TrendStateItem& state : record.states)
				{
					TimeStamp ct = state.getTime(timeType);

					// Break line if it is not valid point
					//
					if (state.isValid() == false)
					{
						if (lines.isEmpty() == false)
						{
							drawPolyline(painter, lines, signalRect);
							lines.clear();
						}

						continue;
					}

					double x = TrendScale::timeToScaledPixel(ct, signalRect, startTimeStamp, duration);
					double y = (state.value == 0) ? yPos0 : yPos1;

					//painter->fillRect(QRectF(x - 1.0/64.0, y - 1.0/64.0, 1.0/32.0, 1.0/32.0), signal.color());
					//drawText(painter, QString("%1").arg(pointIndex), QRectF(x - 1.0/64.0, y - 1.0/64.0, 1.0/32.0, 1.0/32.0), drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip);
					//qDebug() << "pointIndex: " << pointIndex;
					//pointIndex ++;

					if (lines.isEmpty() == true)
					{
						lines.push_back(QPointF(x, y));

						lastX = x;
						lastY = y;
					}
					else
					{
						if (x != lastX || y != lastY)		// If prev point the same, don't add this point
						{
							if (lastY == y)
							{
								lines.push_back(QPointF(x, y));
							}
							else
							{
								lines.push_back(QPointF(x, lastY));
								lines.push_back(QPointF(x, y));
							}

							lastX = x;
							lastY = y;
						}
					}

					if (lastX >= rectRight)
					{
						break;		// end of drawing
					}

				}	// for (const TrendStateItem& state : record.states)

				if (lines.size() >= recomendedSize)
				{
					drawPolyline(painter, lines, signalRect);

					QPointF lastPoint = lines.back();
					lines.clear();
					lines.push_back(lastPoint);
				}

				if (lastX >= rectRight)
				{
					break;		// end of drawing
				}
			}

			if (lastX >= rectRight)
			{
				break;		// end of drawing
			}
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

	void Trend::drawSignalTrendAnalog(QPainter* painter, const TrendSignalParam& signal, const TrendParam& drawParam, const std::list<std::shared_ptr<OneHourData>>& signalData) const
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
					 (signal.lineWeight() <= 1.0) ? drawParam.cosmeticPenWidth() : signal.lineWeight() / drawParam.dpiY());
		painter->setPen(linePen);

		static const int recomendedSize = 8192;
		QVector<QPointF> lines;
		lines.reserve(recomendedSize);

		TimeStamp startTimeStamp = drawParam.startTimeStamp();
		qint64 duration = drawParam.duration();

		double rectRight = signalRect.right();

		double lastX = 0;
		double lastY = 0;

		//		int pointIndex = 0;

		for (std::shared_ptr<OneHourData> hour : signalData)
		{
			const std::vector<TrendStateRecord>& data = hour->data;

			for (const TrendStateRecord& record : data)
			{
				for (const TrendStateItem& state : record.states)
				{
					const TimeStamp& ct = state.getTime(timeType);

					double value = TrendScale::valueToScaleValue(state.value, drawParam.scaleType(), &ok);

					// Break line if it is not valid point or value has wrong value (e.g. logarithm from negative)
					//
					if (state.isValid() == false || ok == false)
					{
						if (lines.isEmpty() == false)
						{
							drawPolyline(painter, lines, signalRect);
							lines.clear();
						}

						continue;
					}

					double x = TrendScale::timeToScaledPixel(ct, signalRect, startTimeStamp, duration);
					double y = TrendScale::valueToScaledPixel(value, signalRect, lowLimit, highLimit);

					//					painter->fillRect(QRectF(x - 1.0/64.0, y - 1.0/64.0, 1.0/32.0, 1.0/32.0), signal.color());
					//					drawText(painter, QString("%1").arg(pointIndex), QRectF(x - 1.0/64.0, y - 1.0/64.0, 1.0/32.0, 1.0/32.0), drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip);
					//					qDebug() << "DEBUG: Discrete draw pointIndex:" << pointIndex
					//							 << ", Flags: " << state.flags
					//							 << ", value: " << state.value
					//							 << ", timestamp: " << ct.toDateTime().toString("HH:mm:ss.zzz");
					//					pointIndex ++;

					if (lines.isEmpty() == true)
					{
						lines.push_back(QPointF(x, y));
						lastX = x;
						lastY = y;
					}
					else
					{
						if (x != lastX || y != lastY)		// If prev point the same, don't add this point
						{
							if (lastY == y)
							{
								if (lines.size() >= 2 &&
									lines[lines.size() - 2].y() == lastY)
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

					if (lastX >= rectRight)
					{
						break;		// end of drawing
					}
				} // for (const TrendStateItem& state : record.states)

				if (lines.size() >= recomendedSize)
				{
					drawPolyline(painter, lines, signalRect);
					QPointF lastPoint = lines.back();
					lines.clear();
					lines.push_back(lastPoint);
				}

				if (lastX >= rectRight)
				{
					break;		// end of drawing
				}
			}	// for (const TrendStateRecord& record : data)

			if (lastX >= rectRight)
			{
				break;		// end of drawing
			}
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

	void Trend::drawRulers(QPainter* painter, const TrendParam& drawParam) const
	{
		if (painter == nullptr)
		{
			Q_ASSERT(painter);
			return;
		}

		adjustPainter(painter, drawParam.dpiX(), drawParam.dpiY());

		double dpiX = static_cast<double>(drawParam.dpiX());
		double dpiY = static_cast<double>(drawParam.dpiY());

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

		if (selectedRulerIndex >= 0 &&
			selectedRulerIndex < static_cast<int>(rulerSet().rulers().size()))
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
			std::sort(laneRulers.begin(), laneRulers.end(),
					  [](const TrendRuler& r1, const TrendRuler& r2)
			{
				return r1.timeStamp() < r2.timeStamp();
			});

			// Calc ruler timestamp text width
			//
			QRectF timeStampBoundRect;
			drawText(painter, " 00:00:00.000 ", QRectF(), drawParam, Qt::AlignCenter, &timeStampBoundRect);

			double rulerTextTop = laneRect.top() + (trendAreaRect.top() - laneRect.top()) / 2.0 - timeStampBoundRect.height() / 2.0;
			double rulerTextHeight = timeStampBoundRect.height();

			// Draw ruler line
			//
			painter->setClipRect(laneRect);

			double k = static_cast<double>(trendAreaRect.width()) / static_cast<double>(drawParam.duration());	// K is coefficient

			for (size_t i = 0; i < laneRulers.size(); i++)
			{
				const TrendRuler& ruler = laneRulers[i];

				if (ruler.timeStamp() < startLaneTime)
				{
					continue;
				}

				double x = trendAreaRect.left() + k * static_cast<double>(ruler.timeStamp().timeStamp - startLaneTime.timeStamp);
				x = static_cast<double>(static_cast<int>(x * dpiX)) / dpiX;		// Ajust x to look nice (not blurred)

				if (ruler.timeStamp() <= finishLaneTime)
				{
					painter->setPen(rulerPen);

					painter->drawLine(QPointF(x, trendAreaRect.top()),
									  QPointF(x, trendAreaRect.bottom()));

					if (ruler.timeStamp() == selectedRulerTime)
					{
						double xx = static_cast<double>(x * dpiX + 1) / dpiX;

						painter->drawLine(QPointF(xx, trendAreaRect.top()),
										  QPointF(xx, trendAreaRect.bottom()));
					}

					// Draw ruler timestamp
					//
					QString text = ruler.timeStamp().toDateTime().toString(" hh:mm:ss.zzz ");
					QRectF textRect(x - timeStampBoundRect.width() / 2.0,
									rulerTextTop,
									timeStampBoundRect.width(),
									rulerTextHeight);

					painter->fillRect(textRect, backgroundBrush);
					drawText(painter, text, textRect, drawParam, Qt::AlignCenter);
				}

				// Draw disctance between rulers
				//
				if (i > 0)
				{
					// There is a previouse ruler, draw distance to it
					//
					const TrendRuler& prevRuler = laneRulers[i - 1];

					double prevRulerX = trendAreaRect.left() + k * static_cast<double>(prevRuler.timeStamp().timeStamp - startLaneTime.timeStamp);
					if (prevRulerX < trendAreaRect.left())
					{
						prevRulerX = trendAreaRect.left();
					}
					else
					{
						prevRulerX += timeStampBoundRect.width() / 2.0;
					}

					double xx = static_cast<double>(static_cast<int>(x * dpiX)) / dpiX;		// Ajust x to look nice (not blurred)
					if (ruler.timeStamp() > finishLaneTime)
					{
						xx = trendAreaRect.right();
					}
					else
					{
						xx -= timeStampBoundRect.width() / 2.0;
					}

					double y = laneRect.top() + (trendAreaRect.top() - laneRect.top()) / 2.0;
					y = static_cast<double>(static_cast<int>(y * dpiY)) / dpiY;		// Ajust x to look nice (not blurred)

					if (prevRulerX < xx)
					{
						painter->setPen(distancePen);
						painter->drawLine(QPointF(prevRulerX, y),
										  QPointF(xx, y));
					}

					// Draw distance between rulers
					//
					qint64 rulersDistance = ruler.timeStamp().timeStamp - prevRuler.timeStamp().timeStamp;
					int msecs = static_cast<int>(rulersDistance % 1000_ms);
					int secs = static_cast<int>(rulersDistance / 1_sec) % 60;
					int mins = static_cast<int>(rulersDistance / 1_min) % 60;
					int hours = static_cast<int>(rulersDistance / 1_hour) % 60;
					int days = static_cast<int>(rulersDistance / 1_day) % 24;

					QString distanceText;

					if (days > 0)
					{
						distanceText = QString::asprintf(" %1dd, %02d:%02d:%02d.%03d ", days, hours, mins, secs, msecs);
					}
					else
					{
						distanceText = QString::asprintf(" %02d:%02d:%02d.%03d ", hours, mins, secs, msecs);
					}

					QRectF distanceTextBoundRect;
					drawText(painter, distanceText, distanceTextBoundRect, drawParam, Qt::AlignCenter, &distanceTextBoundRect);		// Get bound rect

					if (distanceTextBoundRect.width() + distanceTextBoundRect.height() / 2 < xx - prevRulerX)
					{
						QRectF distanceTextRect;
						distanceTextRect.setLeft(prevRulerX + (xx - prevRulerX) / 2.0 - distanceTextBoundRect.width() / 2.0);
						distanceTextRect.setTop(rulerTextTop);
						distanceTextRect.setWidth(distanceTextBoundRect.width());
						distanceTextRect.setHeight(distanceTextBoundRect.height());

						painter->fillRect(distanceTextRect, backgroundBrush);
						drawText(painter, distanceText, distanceTextRect, drawParam, Qt::AlignCenter);			// Draw distanmce bewtween rulers
					}
				}

				if (ruler.timeStamp() > finishLaneTime)
				{
					// Break here, not in the begining of the loop
					// We need to draw this (of lane) ruler, to draw distance to the perv ruler
					//
					break;
				}

				// Draw signals values on the ruler
				//
				if (ruler.timeStamp() >= startLaneTime &&
					ruler.timeStamp() <= finishLaneTime)
				{
					QColor semitransaprentColor = drawParam.backColor2nd();
					semitransaprentColor.setAlpha(200);

					QBrush fillRectBrush(semitransaprentColor);

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

						TrendStateItem state = rulerSignalState(ruler, trendSignal.appSignalId(), timeType);

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
							str = "?";
						}
						else
						{
							str = TrendScale::scaleValueText(state.value, drawParam.scaleType(), trendSignal);
						}

						double vertCoef = (highLimit - lowLimit) / signalRect.height();
						double y = (value - lowLimit) / vertCoef;

						// Get text bounding rect
						//
						QRectF boundRect;
						drawText(painter, str, boundRect, drawParam, Qt::AlignRight | Qt::AlignBottom, &boundRect);

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
								drawRect.setTop(signalRect.bottom() - y /*- boundRect.height() / 2.0*/);	// just below the trend line

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
					}	// End of draw signal values on the ruler
				}
			}	// for (size_t i = 0; i < laneRulers.size(); i++)
		}

		// Reset clipping
		//
		painter->setClipping(false);

		return;
	}

	TrendStateItem Trend::rulerSignalState(const TrendRuler& ruler, QString appSignalId, E::TimeType timeType) const
	{
		const TimeStamp& rulerTime = ruler.timeStamp();

		// Getting data whitout requesting if it is not present
		//
		std::list<std::shared_ptr<OneHourData>> signalData;

		TimeStamp minus1hour(rulerTime.timeStamp - 1_hour);
		TimeStamp plus1hour(rulerTime.timeStamp + 1_hour);

		signalSet().getExistingTrendData(appSignalId, minus1hour.toDateTime(), plus1hour.toDateTime(), timeType, &signalData);

		// Look for state at point ruler.timeStamp
		//
		TrendStateItem lastState;
		lastState.clear();

		for (const std::shared_ptr<OneHourData>& h : signalData)
		{
			const std::vector<TrendStateRecord>& records = h->data;

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
					continue;	// to next record
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
				//						// if currnet state not valid, then we assume last state is not valid also
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
				auto stateIt = std::lower_bound(states.begin(), states.end(), fakeState,
												[&rulerTime, &timeType](const TrendStateItem& state, const TrendStateItem& /*fakseState*/)
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
						if (stateIt == states.begin()) // and it is not equeal to the first (se the prev cond)
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
					Q_ASSERT(false);	// we have checked it before
				}

				Q_ASSERT(false);
			}
		}

		TrendStateItem nonValid;
		nonValid.clear();
		return nonValid;
	}

	void Trend::adjustPainter(QPainter* painter, int dpiX, int dpiY)
	{
		Q_ASSERT(painter);

		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->setRenderHint(QPainter::TextAntialiasing, true);

		painter->resetTransform();

		painter->translate(0.5, 0.5);
		painter->scale(dpiX, dpiY);

		return;
	}

	void Trend::drawPolyline(QPainter* painter, const QVector<QPointF>& lines, const QRectF& rect) const
	{
		Q_ASSERT(painter);

		if (lines.size() < 2)
		{
			return;
		}

		double left = rect.left();
		double right = rect.right();

		if (lines.first().x() > right ||
			lines.last().x() < left)
		{
			return;
		}

		int size = lines.size();
		const QPointF* ptrToFirst = lines.constData();

		int index = 0;
		for (; index < size; index++)
		{
			if (ptrToFirst->x() < left &&
				index < size - 1 &&
				(ptrToFirst + 1)->x() < left)
			{
				ptrToFirst ++;
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
				ptrToLast ++;
			}
			else
			{
				ptrToLast ++;
				break;
			}
		}

		int d = static_cast<int>(std::distance(ptrToFirst, ptrToLast));

		painter->drawPolyline(ptrToFirst, d);

		return;
	}

	void Trend::calcSignalRects(const QRectF& insideRect,
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
				signalRect = QRectF();		// Null rect
			}

			if (signalRect.bottom() > insideRect.bottom())
			{
				signalRect.setBottom(insideRect.bottom());
			}

			// Save calcultaed rect to TrendSignal
			//
			ts.setTempDrawRect(signalRect);
		}

		if (drawParam.viewMode() == TrendViewMode::Separated &&
			analogs->empty() == false)
		{
			const double analogSignalsHeight = qMax((insideRect.bottom() - y) / analogs->size(), discreteSignalHeight);

			for (TrendSignalParam& ts : *analogs)
			{
				QRectF signalRect = {insideRect.left(), y, insideRect.width(), analogSignalsHeight};
				y += analogSignalsHeight;

				if (signalRect.top() >= insideRect.bottom())
				{
					signalRect = QRectF();		// Null rect
				}

				if (signalRect.bottom() > insideRect.bottom())
				{
					signalRect.setBottom(insideRect.bottom());
				}

				// Save calcultaed rect to TrendSignal
				//
				ts.setTempDrawRect(signalRect);
			}
		}

		if (drawParam.viewMode() == TrendViewMode::Overlapped  &&
			analogs->empty() == false)
		{
			const double analogSignalsHeight = qMax(insideRect.bottom() - y, discreteSignalHeight);
			QRectF signalRect = {insideRect.left(), y, insideRect.width(), analogSignalsHeight};

			if (signalRect.top() >= insideRect.bottom())
			{
				signalRect = QRectF();		// Null rect
			}

			if (signalRect.bottom() > insideRect.bottom())
			{
				signalRect.setBottom(insideRect.bottom());
			}

			for (TrendSignalParam& ts : *analogs)
			{
				// Save calcultaed rect to TrendSignal
				//
				ts.setTempDrawRect(signalRect);
			}
		}

		Q_ASSERT(drawParam.viewMode() == TrendViewMode::Separated ||
				 drawParam.viewMode() == TrendViewMode::Overlapped);

		return;
	}

	QRectF Trend::calcLaneRect(int laneIndex, const TrendParam& drawParam)
	{
		QSizeF inchSize(static_cast<double>(drawParam.rect().size().width()) / static_cast<double>(drawParam.dpiX()),
						static_cast<double>(drawParam.rect().size().height()) / static_cast<double>(drawParam.dpiY()));

		double laneMargin = 1.0 / 32.0;		// 1/16 inch
		double laneHeight = (inchSize.height() - laneMargin) / static_cast<double>(drawParam.laneCount()) - laneMargin;

		QRectF laneRect;

		laneRect.setLeft(laneMargin);
		laneRect.setRight(inchSize.width() - laneMargin * 2.0);

		laneRect.setTop(laneMargin + static_cast<double>(laneIndex) * (laneHeight + laneMargin));
		laneRect.setHeight(laneHeight);

		return laneRect;
	}

	QRectF Trend::calcTrendArea(const QRectF& laneRect, const TrendParam& drawParam) const
	{
		int analogsCount = static_cast<int>(signalSet().analogSignalsCount());
		return Trend::calcTrendArea(laneRect, drawParam, analogsCount);
	}

	QRectF Trend::calcTrendArea(const QRectF& laneRect, const TrendParam& drawParam, size_t analogSignalCount)
	{
		double dpiX = drawParam.dpiX();
		double dpiY = drawParam.dpiY();

		// Calc InsideRect(trendArea)
		// +-------------------------------+
		// |  +---------------------------+|
		// |  |   insideRect (trendArea)  ||
		// |  +---------------------------+|
		// +-------------------------------+
		//
		QRectF insideRect;

		if (drawParam.viewMode() == TrendViewMode::Separated)
		{
			insideRect.setLeft(laneRect.left() + 6.0/8.0);
			insideRect.setTop(laneRect.top() + 1.0/5.0);
			insideRect.setWidth(laneRect.width() - insideRect.left() - 2.0/8.0);
			insideRect.setHeight(laneRect.height() - (insideRect.top() - laneRect.top()) - 0.3);
		}

		if (drawParam.viewMode() == TrendViewMode::Overlapped)
		{
			if (analogSignalCount == 0 ||
				analogSignalCount == 1)
			{
				insideRect.setLeft(laneRect.left() + 6.0/8.0);
				insideRect.setTop(laneRect.top() + 1.0/5.0);
				insideRect.setWidth(laneRect.width() - insideRect.left() - 2.0/8.0);
				insideRect.setHeight(laneRect.height() - (insideRect.top() - laneRect.top()) - 0.3);
			}
			else
			{
				insideRect.setLeft(laneRect.left() + 6.0/8.0 * 1.5);
				insideRect.setTop(laneRect.top() + 1.0/5.0);
				insideRect.setWidth(laneRect.width() - insideRect.left() - 2.0/8.0);
				insideRect.setHeight(laneRect.height() - (insideRect.top() - laneRect.top()) - 0.3);
			}
		}

		// Ajust inside rect to dpiX, so it will look pretty while drawing it with cosmetic pen
		//
		insideRect.setLeft(static_cast<double>(static_cast<int>(insideRect.left() * dpiX)) / dpiX);
		insideRect.setTop(static_cast<double>(static_cast<int>(insideRect.top() * dpiY)) / dpiY);
		insideRect.setWidth(static_cast<double>(static_cast<int>(insideRect.width() * dpiX)) / dpiX);
		insideRect.setHeight(static_cast<double>(static_cast<int>(insideRect.height() * dpiY)) / dpiY);

		return insideRect;
	}

	QRectF Trend::calcScaleAreaRect(const QRectF& laneRect, const QRectF& signalRect)
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
		QRectF result(QPointF(laneRect.left(), signalRect.top()),
					  QPointF(signalRect.left(), signalRect.bottom()));

		return result;
	}

	QRect Trend::inchRectToPixelRect(const QRectF& rect, const TrendParam& drawParam)
	{
		QRect result(static_cast<int>(rect.left() * drawParam.dpiX()),
					 static_cast<int>(rect.top() * drawParam.dpiY()),
					 static_cast<int>(rect.width() * drawParam.dpiX()),
					 static_cast<int>(rect.height() * drawParam.dpiY()));

		return result;
	}

	QRectF Trend::pixelRectToInchRect(const QRect& rect, const TrendParam& drawParam)
	{
		QRectF result(static_cast<double>(rect.left()) / static_cast<double>(drawParam.dpiX()),
					  static_cast<double>(rect.top()) / static_cast<double>(drawParam.dpiY()),
					  static_cast<double>(rect.width()) / static_cast<double>(drawParam.dpiX()),
					  static_cast<double>(rect.height()) / static_cast<double>(drawParam.dpiY()));

		return result;
	}

	QPoint Trend::inchPointToPixelPoint(const QPointF& point, const TrendParam& drawParam)
	{
		QPoint result(static_cast<int>(point.x() * drawParam.dpiX()),
					  static_cast<int>(point.y() * drawParam.dpiY()));
		return result;
	}

	QPointF Trend::pixelPointToInchPoint(const QPoint& point, const TrendParam& drawParam)
	{
		QPointF result(static_cast<double>(point.x()) / static_cast<double>(drawParam.dpiX()),
					   static_cast<double>(point.y()) / static_cast<double>(drawParam.dpiY()));
		return result;
	}

	Trend::MouseOn Trend::mouseIsOver(QPoint mousePos, const TrendParam& drawParam, int* outLaneIndex, TimeStamp* outTime, int* rulerIndex, TrendSignalParam* outSignal) const
	{
		if (outLaneIndex == nullptr ||
			outTime == nullptr ||
			rulerIndex == nullptr ||
			outSignal == nullptr)
		{
			Q_ASSERT(outLaneIndex);
			Q_ASSERT(outTime);
			Q_ASSERT(rulerIndex);
			Q_ASSERT(outSignal);
			return Trend::MouseOn::Outside;
		}

		*outLaneIndex = -1;
		*rulerIndex = -1;
		*outTime = TimeStamp();

		QRectF rect = drawParam.rect();
		QPointF pos(static_cast<double>(mousePos.x()) / static_cast<double>(drawParam.dpiX()),		// Transform mousePos to inches, as everything for drawing is done in inches
					static_cast<double>(mousePos.y()) / static_cast<double>(drawParam.dpiY()));

		// MouseOn::Outside
		//
		if (rect.contains(mousePos) == false)
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
					for (const std::pair<QString, QRectF>& p : drawParam.signalDescriptionRect())
					{
						if (p.second.contains(pos) == true)
						{
							QString signalId = p.first;

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
		return MouseOn::Outside;		// Can be frame beetween lanes
	}

	void Trend::drawText(QPainter* painter, const QString& str, const QRectF& rect, const TrendParam& drawParam, int flags, QRectF* boundingRect/* = nullptr*/)
	{
		if (painter == nullptr)
		{
			Q_ASSERT(painter);
			return;
		}

		if (str.isEmpty())
		{
			return;
		}

		painter->save();

		double dpiX = drawParam.dpiX();
		double dpiY = drawParam.dpiY();

		painter->scale(1.0 / dpiX, 1.0 / dpiY);

		QFont f(QStringLiteral("Arial"));

		double fontSize = 2.0 / 16.0;	// Font size is 2/16 in
		if (dpiY > 120)
		{
			fontSize = 1.75 / 16.0;		// For HiDPI
		}

		int pixelSize = qRound(fontSize * dpiY);
		f.setPixelSize(pixelSize > 0 ? pixelSize : 1);
		painter->setFont(f);

		QRectF rc;
		rc.setLeft(rect.left() * dpiX);
		rc.setTop(rect.top() * dpiY);
		rc.setRight(rect.right() * dpiX);
		rc.setBottom(rect.bottom() * dpiY);

		QRectF boundingRectIn;
		painter->drawText(rc, flags, str, &boundingRectIn);

		if (boundingRect != nullptr)
		{
			*boundingRect = QRectF(boundingRectIn.left() / dpiX, boundingRectIn.top() / dpiY, boundingRectIn.width() / dpiX, boundingRectIn.height() / dpiY);
		}

		painter->restore();
		return;
	}

	TrendLib::TrendSignalSet& Trend::signalSet()
	{
		return m_signalSet;
	}

	const TrendLib::TrendSignalSet& Trend::signalSet() const
	{
		return m_signalSet;
	}

	TrendLib::TrendRulerSet& Trend::rulerSet()
	{
		return m_rulerSet;
	}

	const TrendLib::TrendRulerSet& Trend::rulerSet() const
	{
		return m_rulerSet;
	}

}
