#include "Trend.h"
#include "QPainter"

namespace TrendLib
{

	Trend::Trend()
	{
	}

	void Trend::draw(QImage* image, const TrendDrawParam& drawParam) const
	{
		if (image == nullptr)
		{
			assert(image);
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

	void Trend::draw(QPainter* painter, const TrendDrawParam& drawParam, bool needAdjustPainter) const
	{
		assert(painter);

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

			TrendDrawParam laneDrawParam = drawParam;
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

	void Trend::drawLane(QPainter* painter, const QRectF& laneRect, const TrendDrawParam& drawParam) const
	{
		painter->setBrush(drawParam.backColor1st());
		painter->setPen(Qt::PenStyle::NoPen);
		painter->drawRect(laneRect);

		// Calc InsideRect
		// +-------------------------------+
		// |  +---------------------------+|
		// |  |   insideRect (trendArea)  ||
		// |  +---------------------------+|
		// +-------------------------------+
		//
		QRectF insideRect = calcTrendArea(laneRect, drawParam);

		std::vector<TrendSignalParam> discretes = signalSet().discreteSignals();
		std::vector<TrendSignalParam> analogs = signalSet().analogSignals();

		// Calc signals rects, calculates rect will be written to discretes/analogs
		//
		calcSignalRects(painter, insideRect, drawParam, &discretes, &analogs);

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

//		double y = insideRect.top();
//		QColor signalBackColor = drawParam.backgroundColor();

//		// Draw discrete signals is the same for both modes (TrendView::Separated/TrendView::Overlapped)
//		//
//		for (int signalIndex = 0; signalIndex < static_cast<int>(discreteSignals.size()); signalIndex++)
//		{
//			const TrendSignalParam& s = discreteSignals[signalIndex];

//			QRectF signalRect = {insideRect.left(), y, insideRect.width(), discreteSignalHeight};
//			y += discreteSignalHeight;

//			if (signalRect.top() >= insideRect.bottom())
//			{
//				break;
//			}

//			if (signalRect.bottom() > insideRect.bottom())
//			{
//				signalRect.setBottom(insideRect.bottom());
//			}

//			signalBackColor = (signalBackColor == drawParam.laneBackgroundColor()) ? drawParam.backgroundColor() : drawParam.laneBackgroundColor();
//			painter->fillRect(signalRect, signalBackColor);

//			drawSignal(painter, s, signalIndex, signalRect, drawParam, signalBackColor);
//		}

//		// Specific drawing for analog signals
//		//
//		if (drawParam.view() == TrendView::Separated)
//		{
//			const double analogSignalsHeight = qMax((insideRect.bottom() - y) / analogSignals.size(), discreteSignalHeight);

//			for (int signalIndex = 0; signalIndex < static_cast<int>(analogSignals.size()); signalIndex++)
//			{
//				const TrendSignalParam& s = analogSignals[signalIndex];

//				QRectF signalRect = {insideRect.left(), y, insideRect.width(), analogSignalsHeight};
//				y += analogSignalsHeight;

//				if (signalRect.top() >= insideRect.bottom())
//				{
//					break;
//				}

//				if (signalRect.bottom() > insideRect.bottom())
//				{
//					signalRect.setBottom(insideRect.bottom());
//				}

//				signalBackColor = (signalBackColor == drawParam.laneBackgroundColor()) ? drawParam.backgroundColor() : drawParam.laneBackgroundColor();
//				painter->fillRect(signalRect, signalBackColor);

//				drawSignal(painter, s, signalIndex, signalRect, drawParam, signalBackColor);
//			}
//		}

//		if (drawParam.view() == TrendView::Overlapped &&
//			y < insideRect.bottom())
//		{
//			const double analogSignalsHeight = qMax(insideRect.bottom() - y, discreteSignalHeight);
//			QRectF signalRect = {insideRect.left(), y, insideRect.width(), analogSignalsHeight};

//			if (signalRect.bottom() > insideRect.bottom())
//			{
//				signalRect.setBottom(insideRect.bottom());
//			}

//			signalBackColor = (signalBackColor == drawParam.laneBackgroundColor()) ? drawParam.backgroundColor() : drawParam.laneBackgroundColor();
//			painter->fillRect(signalRect, signalBackColor);

//			for (int signalIndex = 0; signalIndex < static_cast<int>(analogSignals.size()); signalIndex++)
//			{
//				const TrendSignalParam& s = analogSignals[signalIndex];

//				drawSignal(painter, s, signalIndex, signalRect, drawParam, signalBackColor);
//			}
//		}

//		// Draw insideRect
//		//
//		QPen insideRectPen(Qt::darkGray, drawParam.cosmeticPenWidth(), Qt::SolidLine);
//		painter->setPen(insideRectPen);
//		painter->setBrush(Qt::BrushStyle::NoBrush);

//		painter->drawRect(insideRect);


		return;
	}

	void Trend::drawBackground(QPainter* painter,
							   const QRectF& insideRect,
							   const TrendDrawParam& drawParam,
							   const std::vector<TrendSignalParam>& discretes,
							   const std::vector<TrendSignalParam>& analogs) const
	{
		assert(painter);
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
		assert(drawParam.view() == TrendView::Separated ||
			   drawParam.view() == TrendView::Overlapped);

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

		if (drawParam.view() == TrendView::Separated  &&
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

		if (drawParam.view() == TrendView::Overlapped &&
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

	void Trend::drawTimeGrid(QPainter* painter, const QRectF& laneRect, const QRectF& insideRect, const TrendDrawParam& drawParam) const
	{
		double dpiX = drawParam.dpiX();

		// Calc time grid
		//
		static const std::array<qint64, 29> possibleTimeGridIntervals = {10_ms, 20_ms, 25_ms, 50_ms,
																		 100_ms, 200_ms, 250_ms, 500_ms,
																		 1_sec, 2_sec, 5_sec, 10_sec, 15_sec, 20_sec, 30_sec,
																		 1_min, 90_sec, 2_min, 5_min, 10_min, 15_min, 20_min, 30_min,
																		 1_hour, 2_hours, 3_hours, 6_hours, 12_hours, 24_hours};

		QRectF boundRect;
		QString estimatedString = (drawParam.duration() < static_cast<quint64>(10_sec)) ? "HH:MM:SS.XXX" : "HH:MM:SS";
		drawText(painter, estimatedString, QRectF(), drawParam, Qt::AlignCenter, &boundRect);

		double minTimeInterval = boundRect.width() * 1.33;
		//double minTimeInterval = 3.0/4.0;	// 3/4 in -- minimum inches interval

		TimeStamp startTimeStamp = drawParam.startTimeStamp();
		qint64 duration = drawParam.duration();

		qint64 timeGridInterval = possibleTimeGridIntervals[possibleTimeGridIntervals.size() - 1];
		double inchGridInterval = 0;

		for (qint64 possibleInterval : possibleTimeGridIntervals)
		{
			TimeStamp ct = TimeStamp{startTimeStamp.timeStamp + possibleInterval};

			double x = timeToScaledPixel(ct, insideRect, startTimeStamp, duration);

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
			double x = timeToScaledPixel(ct, insideRect, startTimeStamp, duration);

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
				time == QTime(0, 0, 0, 0))
			{
				lastDate = date;

				double x = static_cast<double>(p.x * dpiX + 1) / dpiX;

				QPointF pt1(x, insideRect.top());
				QPointF pt2(x, insideRect.bottom());
				painter->drawLine(pt1, pt2);
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
			QString timeText = p.timeStamp.toDateTime().toString(timeGridInterval < 1_sec ? "hh:mm:ss.zzz" : "hh:mm:ss");
			QString dateText = p.timeStamp.toDateTime().toString("dd.MM.yyyy");

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
								 const TrendDrawParam& drawParam,
								 const std::vector<TrendSignalParam>& discretes,
								 const std::vector<TrendSignalParam>& analogs) const
	{
		assert(painter);
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
		assert(drawParam.view() == TrendView::Separated ||
			   drawParam.view() == TrendView::Overlapped);

		if (drawParam.view() == TrendView::Separated  &&
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

				painter->setPen(ts.color());

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
		if (drawParam.view() == TrendView::Overlapped  &&
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

				painter->setPen(ts.color());

				QRectF testDesctriptionBoundRect;
				drawText(painter, signalText, signalRect, drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, &testDesctriptionBoundRect);

				auto scr = std::make_pair(ts.appSignalId(), testDesctriptionBoundRect);
				drawParam.signalDescriptionRect().push_back(scr);

				// Shift rect
				//
				signalRect.setTop(testDesctriptionBoundRect.bottom() + testDesctriptionBoundRect.height() * 1.2);
			}
		}

//		// --
//		//
		painter->setClipping(false);

		return;
	}

	void Trend::drawAnalogSignalsGridSeparateMode(QPainter* painter,
												  const QRectF& laneRect,
												  const TrendDrawParam& drawParam,
												  const TrendSignalParam& signal) const
	{
		assert(painter);
		painter->setClipping(false);

		QRectF signalRect = signal.tempDrawRect();
		QRectF scaleAreaRect = calcScaleAreaRect(laneRect, signalRect);

		if (signalRect.isEmpty() == true ||
			scaleAreaRect.isEmpty() == true)
		{
			return;
		}

		double highLimit = qMax(signal.viewHighLimit(), signal.viewLowLimit());
		double lowLimit = qMin(signal.viewHighLimit(), signal.viewLowLimit());

		double delta = highLimit - lowLimit;
		if (delta <= DBL_MIN)
		{
			// Divide by 0 possible
			//
			return;
		}

		double dpiY = drawParam.dpiY();

		// Calc vert grid
		//
		static const std::array<double, 4> possibleGridIntervals = {0.1, 0.2, 0.25, 0.5};

		double minInchInterval = 1.0/4.0;	// 1/4 in -- minimum inches interval
		double gridValue = 1.0;

		double pow = 1e-30;
		for (int mult = 0; mult <= 60; mult++, pow *= 10.0)
		{
			for (size_t i = 0; i < possibleGridIntervals.size(); i++)
			{
				gridValue = possibleGridIntervals[i] * pow;

				double y = valueToScaledPixel(lowLimit + gridValue, signalRect, lowLimit, highLimit);
				if (signalRect.bottom() - y >= minInchInterval)
				{
					// gridValue contains found suitable value for grid
					//
					//qDebug() << "GridValue " << gridValue << ", distance in inches " << rect.bottom() - y ;
					mult = 1000000;		// To break outer loop
					break;
				}
			}
		}

		// Align gridValue
		//
		double lowGriddedValue = floor(lowLimit / gridValue) * gridValue;
		int gridCount = static_cast<int>(delta / gridValue) + 2;

		if (gridCount < 0 || gridCount > 100)
		{
			// Something wrong
			//
			assert(false);
			return;
		}

		// Draw horz grids
		//
		QPen gridPen(Qt::lightGray, drawParam.cosmeticPenWidth(), Qt::PenStyle::DashLine);
		painter->setPen(gridPen);

		std::vector<std::pair<double, double>> grids;		// first: y pos, second: value
		grids.reserve(gridCount);

		for (int i = 0; i < gridCount; i++)
		{
			double value = lowGriddedValue + i * gridValue;

			double y = valueToScaledPixel(value, signalRect, lowLimit, highLimit);
			y = static_cast<double>(static_cast<int>(y * dpiY)) / dpiY;		// Align to DPI

			if (y < signalRect.top() ||
				y > signalRect.bottom())
			{
				continue;
			}

			painter->drawLine(QPointF(signalRect.left(), y),
							  QPointF(signalRect.right(), y));

			grids.emplace_back(y, value);
		}

		// Draw grid values
		//
		painter->setPen(signal.color());

		QRectF boundTextRect;
		drawText(painter, "0", QRectF(), drawParam, Qt::AlignCenter, &boundTextRect);
		double textHeight = boundTextRect.height();

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

			QString text = QString(" %1 ").arg(QString::number(value, 'g'));
			drawText(painter, text, textRect, drawParam, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextDontClip);
		}

		painter->setClipping(false);
		return;
	}

	void Trend::drawSignalTrend(QPainter* painter, const TrendSignalParam& signal, const TrendDrawParam& drawParam) const
	{
		assert(painter);

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

	void Trend::drawSignalTrendDiscrete(QPainter* painter, const TrendSignalParam& signal, const TrendDrawParam& drawParam, const std::list<std::shared_ptr<OneHourData>>& signalData) const
	{
		assert(painter);
		assert(signal.isDiscrete() == true);

		QRectF signalRect = signal.tempDrawRect();

		// Set clip region
		//
		painter->setClipRect(signalRect);

		// Draw trend
		//
		QRectF textBoundRect;
		drawText(painter, "0", textBoundRect, drawParam, Qt::AlignLeft | Qt::AlignTop, &textBoundRect);

		TimeType timeType = drawParam.timeType();

		QPen linePen(signal.color(), drawParam.cosmeticPenWidth(), Qt::SolidLine);
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
					if (state.isValid() == false &&
						lines.isEmpty() == false)
					{
						drawPolyline(painter, lines, signalRect);
						lines.clear();
						continue;
					}

					double x = timeToScaledPixel(ct, signalRect, startTimeStamp, duration);
					double y = (state.value == 0) ? yPos0 : yPos1;

//					painter->fillRect(QRectF(x - 1.0/64.0, y - 1.0/64.0, 1.0/32.0, 1.0/32.0), signal.color());
//					drawText(painter, QString("%1").arg(pointIndex), QRectF(x - 1.0/64.0, y - 1.0/64.0, 1.0/32.0, 1.0/32.0), drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip);
//					qDebug() << "DEBUG: Discrete draw pointIndex: " << pointIndex;
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
					lines.clear();
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

	void Trend::drawSignalTrendAnalog(QPainter* painter, const TrendSignalParam& signal, const TrendDrawParam& drawParam, const std::list<std::shared_ptr<OneHourData>>& signalData) const
	{
		assert(painter);
		assert(signal.isAnalog() == true);

		QRectF signalRect = signal.tempDrawRect();

		// Set clip region
		//
		painter->setClipRect(signalRect);

		painter->setRenderHint(QPainter::Antialiasing, false);

		// Draw trend
		//
		double highLimit = qMax(signal.viewHighLimit(), signal.viewLowLimit());
		double lowLimit = qMin(signal.viewHighLimit(), signal.viewLowLimit());

		double delta = highLimit - lowLimit;
		if (delta <= DBL_MIN)
		{
			// Divide by 0 possible
			//
			return;
		}

		TimeType timeType = drawParam.timeType();

		QPen linePen(signal.color(), drawParam.cosmeticPenWidth());
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

					// Break line if it is not valid point
					//
					if (state.isValid() == false &&
						lines.isEmpty() == false)
					{
						drawPolyline(painter, lines, signalRect);
						lines.clear();
						continue;
					}

					double x = timeToScaledPixel(ct, signalRect, startTimeStamp, duration);
					double y = valueToScaledPixel(state.value, signalRect, lowLimit, highLimit);

//					painter->fillRect(QRectF(x - 1.0/64.0, y - 1.0/64.0, 1.0/32.0, 1.0/32.0), signal.color());
//					drawText(painter, QString("%1").arg(pointIndex), QRectF(x - 1.0/64.0, y - 1.0/64.0, 1.0/32.0, 1.0/32.0), drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextDontClip);
//					qDebug() << "DEBUG: Discrete draw pointIndex:" << pointIndex << ", Flags: " << state.flags << ", value: " << state.value;
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

	void Trend::drawSignal(QPainter* painter, const TrendSignalParam& signal, int signalIndex, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor) const
	{
//		assert(painter);

//		// --
//		//
//		painter->setPen(signal.color());

//		// Set clipo region, as SignalID and caption can fo out of drawArea
//		//
//		painter->setClipRect(rect);

//		// --
//		//
//		QString signalText;
//		QRectF testDesctriptionBoundRect = QRectF(rect.topLeft(), rect.topLeft());

//		if (signal.isDiscrete() == true)
//		{
//			signalText = QString("  %1 - %2").arg(signal.signalId()).arg(signal.caption());

//			drawText(painter, signalText, testDesctriptionBoundRect, drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine  | Qt::TextDontClip, &testDesctriptionBoundRect);
//		}

//		if (signal.isAnalog() == true)
//		{
//			if (signal.unit().isEmpty() == true)
//			{
//				signalText = QString("  %1 - %2").arg(signal.signalId()).arg(signal.caption());
//			}
//			else
//			{
//				signalText = QString("  %1 - %2, %3").arg(signal.signalId()).arg(signal.caption()).arg(signal.unit());
//			}

//			drawText(painter, signalText, testDesctriptionBoundRect, drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine, &testDesctriptionBoundRect);

//			// Shift rect taking signaIndex for Overlapped mode
//			//
//			if (drawParam.view() == TrendView::Overlapped)
//			{
//				testDesctriptionBoundRect.setTop(testDesctriptionBoundRect.top() + testDesctriptionBoundRect.height() * 1.5 * signalIndex);
//			}

//			drawText(painter, signalText, testDesctriptionBoundRect, drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine | Qt::TextDontClip, &testDesctriptionBoundRect);
//		}

//		auto a = std::make_pair(signal.appSignalId(), testDesctriptionBoundRect);
//		std::vector<std::pair<QString, QRectF>>& b = drawParam.signalDescriptionRect();
//		b.push_back(a);

//		painter->setClipping(false);	// Restore clip region

//		// Get signal data
//		//
//		QDateTime startTime = drawParam.startTime();
//		QDateTime finishTime = TimeStamp(drawParam.startTimeStamp().timeStamp + drawParam.duration()).toDateTime();

//		std::list<std::shared_ptr<OneHourData>> signalData;

//		bool requestResult = signalSet().getTrendData(signal.appSignalId(), startTime, finishTime, drawParam.timeType(), &signalData);
//		if (requestResult == false)
//		{
//			signalData.clear();
//		}

//		// Draw signal trend
//		//
//		if (signal.isDiscrete() == true)
//		{
//			drawDiscrete(painter, signal, rect, drawParam, backColor, signalData);
//		}
//		else
//		{
//			drawAnalog(painter, signal, signalIndex, rect, drawParam, backColor, signalData);
//		}

		return;
	}

	void Trend::drawDiscrete(QPainter* painter, const TrendSignalParam& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor, const std::list<std::shared_ptr<OneHourData> >& signalData) const
	{
//		assert(painter);
//		assert(signal.isDiscrete() == true);

//		// Draw units (0, 1) on the left side of rect
//		//
//		painter->setPen(signal.color());

//		QRectF textBoundRect;
//		drawText(painter, "0 ", QRectF(rect.left(), rect.bottom(), 0, 0), drawParam, Qt::AlignRight | Qt::AlignBottom | Qt::TextDontClip, &textBoundRect);	// Get bound rect, for understaning text height
//		drawText(painter, "1 ", QRectF(rect.left(), rect.top() + textBoundRect.height(), 0, 0), drawParam, Qt::AlignRight | Qt::AlignVCenter | Qt::TextDontClip);

//		// Set clip region
//		//
//		painter->setClipRect(rect);

//		// Draw trend
//		//
//		TimeType timeType = drawParam.timeType();

//		QPen linePen(signal.color(), drawParam.cosmeticPenWidth(), Qt::SolidLine);
//		painter->setPen(linePen);

//		static const int recomendedSize = 8192;
//		QVector<QPointF> lines;
//		lines.reserve(recomendedSize);

//		TimeStamp startTimeStamp = drawParam.startTimeStamp();
//		qint64 duration = drawParam.duration();

//		double dpiY = drawParam.dpiY();

//		double yPos0 = rect.bottom() - textBoundRect.height() / 2.0;
//		double yPos1 = rect.top() + textBoundRect.height() * 1.1;
//		yPos0 = static_cast<double>(static_cast<int>(yPos0 * dpiY)) / dpiY;		// Make sure that Y is proper alligned for nice look of cosmetic pen
//		yPos1 = static_cast<double>(static_cast<int>(yPos1 * dpiY)) / dpiY;		// Make sure that Y is proper alligned for nice look of cosmetic pen

//		double rectRight = rect.right();

//		double lastX = 0;
//		double lastY = 0;

//		for (std::shared_ptr<OneHourData> hour : signalData)
//		{
//			const std::vector<TrendStateRecord>& data = hour->data;

//			for (const TrendStateRecord& record : data)
//			{
//				for (const TrendStateItem& state : record.states)
//				{
//					TimeStamp ct = state.getTime(timeType);

//					// Break line if it is not valid point
//					//
//					if (state.isValid() == false &&
//						lines.isEmpty() == false)
//					{
//						painter->drawPolyline(lines);
//						lines.clear();
//						continue;
//					}

//					double x = timeToScaledPixel(ct, rect, startTimeStamp, duration);
//					double y = (state.value == 0) ? yPos0 : yPos1;

//					if (lines.isEmpty() == true)
//					{
//						lines.push_back(QPointF(x, y));

//						lastX = x;
//						lastY = y;
//					}
//					else
//					{
//						if (x != lastX || y != lastY)		// If prev point the same, don't add this point
//						{
//							if (lastY == y)
//							{
//								if (lines.size() > 1)
//								{
//									// Just extend the last line
//									//
//									lines.back().rx() = x;
//								}
//								else
//								{
//									lines.push_back(QPointF(x, y));
//								}
//							}
//							else
//							{
//								// Create another curve on line
//								//
//								lines.push_back(QPointF(x, lastY));
//								lines.push_back(QPointF(x, y));
//							}

//							lastX = x;
//							lastY = y;
//						}
//					}

//					if (lastX >= rectRight)
//					{
//						break;		// end of drawing
//					}

//				}	// for (const TrendStateItem& state : record.states)

//				if (lines.size() >= recomendedSize)
//				{
//					//painter->drawPolyline(lines);
//					drawPolyline(painter, lines, rect);
//					lines.clear();
//				}

//				if (lastX >= rectRight)
//				{
//					break;		// end of drawing
//				}
//			}

//			if (lastX >= rectRight)
//			{
//				break;		// end of drawing
//			}
//		}

//		if (lines.size() >= 2)
//		{
//			painter->drawPolyline(lines);
//			lines.clear();
//		}

//		// Reset clipping
//		//
//		painter->setClipping(false);

		return;
	}

	void Trend::drawAnalog(QPainter* painter, const TrendSignalParam& signal, int signalIndex, const QRectF& rect, const TrendDrawParam& drawParam, QColor /*backColor*/, const std::list<std::shared_ptr<OneHourData>>& signalData) const
	{
//		assert(painter);
//		assert(signal.isAnalog() == true);

//		// Set clip region
//		//
//		//painter->setClipRect(rect);

//		double highLimit = qMax(signal.viewHighLimit(), signal.viewLowLimit());
//		double lowLimit = qMin(signal.viewHighLimit(), signal.viewLowLimit());

//		double delta = highLimit - lowLimit;
//		if (delta <= DBL_MIN)
//		{
//			// Divide by 0 possible
//			//
//			return;
//		}

//		// Draw scale and grid
//		//
//		drawAnalogTimeGrid(painter, signal, signalIndex, rect, drawParam);

//		// Set clip region
//		//
//		painter->setClipRect(rect);

//		// Draw trend
//		//
//		TimeType timeType = drawParam.timeType();

//		QPen linePen(signal.color(), drawParam.cosmeticPenWidth());
//		painter->setPen(linePen);

//		static const int recomendedSize = 8192;
//		QVector<QPointF> lines;
//		lines.reserve(recomendedSize);

//		TimeStamp startTimeStamp = drawParam.startTimeStamp();
//		qint64 duration = drawParam.duration();

//		double rectRight = rect.right();

//		double lastX = 0;
//		double lastY = 0;

//		for (std::shared_ptr<OneHourData> hour : signalData)
//		{
//			const std::vector<TrendStateRecord>& data = hour->data;

//			for (const TrendStateRecord& record : data)
//			{
//				for (const TrendStateItem& state : record.states)
//				{
//					const TimeStamp& ct = state.getTime(timeType);

//					// Break line if it is not valid point
//					//
//					if (state.isValid() == false &&
//						lines.isEmpty() == false)
//					{
//						drawPolyline(painter, lines, rect);
//						lines.clear();
//						continue;
//					}

//					double x = timeToScaledPixel(ct, rect, startTimeStamp, duration);
//					double y = valueToScaledPixel(state.value, rect, lowLimit, highLimit);

//					if (lines.isEmpty() == true)
//					{
//						lines.push_back(QPointF(x, y));
//						lastX = x;
//						lastY = y;
//					}
//					else
//					{
//						if (x != lastX || y != lastY)		// If prev point the same, don't add this point
//						{
//							if (lastY == y)
//							{
//								if (lines.size() > 1)
//								{
//									// Just extend the last line
//									//
//									lines.back().rx() = x;
//								}
//								else
//								{
//									lines.push_back(QPointF(x, y));
//								}
//							}
//							else
//							{
//								// Create another curve on line
//								//
//								lines.push_back(QPointF(x, lastY));
//								lines.push_back(QPointF(x, y));
//							}

//							lastX = x;
//							lastY = y;
//						}

//					}

//					if (lastX >= rectRight)
//					{
//						break;		// end of drawing
//					}

//				} // for (const TrendStateItem& state : record.states)

//				if (lines.size() >= recomendedSize)
//				{
//					drawPolyline(painter, lines, rect);
//					lines.clear();
//				}

//				if (lastX >= rectRight)
//				{
//					break;		// end of drawing
//				}
//			}	// for (const TrendStateRecord& record : data)

//			if (lastX >= rectRight)
//			{
//				break;		// end of drawing
//			}
//		}

//		if (lines.size() >= 2)
//		{
//			drawPolyline(painter, lines, rect);
//			lines.clear();
//		}

//		// Reset clipping
//		//
//		painter->setClipping(false);

		return;
	}

	void Trend::drawAnalogTimeGrid(QPainter* painter,
								   const TrendSignalParam& signal,
								   int signalIndex,
								   const QRectF& rect,
								   const TrendDrawParam& drawParam) const
	{
//		double highLimit = qMax(signal.viewHighLimit(), signal.viewLowLimit());
//		double lowLimit = qMin(signal.viewHighLimit(), signal.viewLowLimit());

//		double delta = highLimit - lowLimit;
//		if (delta <= DBL_MIN)
//		{
//			// Divide by 0 possible
//			//
//			return;
//		}

//		double dpiY = drawParam.dpiY();

//		// Calc vert grid
//		//
//		static const std::array<double, 4> possibleGridIntervals = {0.1, 0.2, 0.25, 0.5};

//		double minInchInterval = 1.0/4.0;	// 1/4 in -- minimum inches interval
//		double gridValue = 1.0;

//		double pow = 1e-30;
//		for (int mult = 0; mult <= 60; mult++, pow *= 10.0)
//		{
//			for (size_t i = 0; i < possibleGridIntervals.size(); i++)
//			{
//				gridValue = possibleGridIntervals[i] * pow;

//				double y = valueToScaledPixel(lowLimit + gridValue, rect, lowLimit, highLimit);
//				if (rect.bottom() - y >= minInchInterval)
//				{
//					// gridValue contains found suitable value for grid
//					//
//					//qDebug() << "GridValue " << gridValue << ", distance in inches " << rect.bottom() - y ;
//					mult = 1000000;		// To break outer loop
//					break;
//				}
//			}
//		}

//		// Align gridValue
//		//
//		double lowGriddedValue = floor(lowLimit / gridValue) * gridValue;
//		int gridCount = static_cast<int>(delta / gridValue) + 2;

//		if (gridCount < 0 || gridCount > 100)
//		{
//			// Something wrong
//			//
//			assert(false);
//			return;
//		}

//		// Draw time grid
//		//
//		QPen gridPen(Qt::lightGray, drawParam.cosmeticPenWidth(), Qt::PenStyle::DashLine);
//		painter->setPen(gridPen);

//		std::vector<std::pair<double, double>> grids;		// first: y pos, second: value
//		grids.reserve(gridCount);

//		for (int i = 0; i < gridCount; i++)
//		{
//			double value = lowGriddedValue + i * gridValue;

//			double y = valueToScaledPixel(value, rect, lowLimit, highLimit);
//			y = static_cast<double>(static_cast<int>(y * dpiY)) / dpiY;		// Align to DPI

//			if (y < rect.top() ||
//				y > rect.bottom())
//			{
//				continue;
//			}

//			painter->drawLine(QPointF(rect.left(), y),
//							  QPointF(rect.right(), y));

//			grids.emplace_back(y, value);
//		}

//		// Draw grid values
//		//
//		painter->setPen(signal.color());

//		QRectF boundTextRect;
//		drawText(painter, "0", QRectF(), drawParam, Qt::AlignCenter, &boundTextRect);
//		double textHeight = boundTextRect.height();

//		for (const std::pair<double, double>& p : grids)
//		{
//			double y = p.first;
//			double value = p.second;

//			QRectF textRect(rect.left(), y - textHeight / 2.0, 0, textHeight);

//			QString text = QString(" %1 ").arg(QString::number(value, 'g'));
//			drawText(painter, text, textRect, drawParam, Qt::AlignRight | Qt::AlignVCenter | Qt::TextDontClip);
//		}

	}

	void Trend::drawRullers(QPainter* painter, const TrendDrawParam& drawParam) const
	{
		if (painter == nullptr)
		{
			assert(painter);
			return;
		}

		adjustPainter(painter, drawParam.dpiX(), drawParam.dpiY());

		double dpiX = static_cast<double>(drawParam.dpiX());
		double dpiY = static_cast<double>(drawParam.dpiY());

		// Prepare drawing resources
		//
		QPen rullerPen(QBrush(qRgb(0x00, 0x00, 0xC0)), drawParam.cosmeticPenWidth(), Qt::PenStyle::DashLine);
		QPen distancePen(QBrush(qRgb(0x00, 0x00, 0xC0)), drawParam.cosmeticPenWidth(), Qt::PenStyle::SolidLine);

		QBrush backgroundBrush(drawParam.backColor1st());
		painter->setBrush(backgroundBrush);

		// --
		//
		int selectedRullerIndex = drawParam.hightlightRullerIndex();
		TimeStamp selectedRullerTime;

		if (/*selectedRullerIndex != -1 &&*/
			selectedRullerIndex >= 0 &&
			selectedRullerIndex < static_cast<int>(rullerSet().rullers().size()))
		{
			selectedRullerTime = rullerSet().rullers()[selectedRullerIndex].timeStamp();
		}

		for (int laneIndex = 0; laneIndex < drawParam.laneCount(); laneIndex++)
		{
			TimeStamp startLaneTime = drawParam.startTimeStamp().timeStamp + laneIndex * drawParam.duration();
			TimeStamp finishLaneTime = startLaneTime.timeStamp + drawParam.duration();

			TrendDrawParam laneDrawParam = drawParam;
			laneDrawParam.setStartTimeStamp(startLaneTime);

			QRectF laneRect = calcLaneRect(laneIndex, laneDrawParam);
			QRectF trendAreaRect = calcTrendArea(laneRect, laneDrawParam);

			//std::vector<TrendRuller> laneRullers = rullerSet().getRullers(startLaneTime, finishLaneTime);

			std::vector<TrendRuller> laneRullers = rullerSet().rullers();
			std::sort(laneRullers.begin(), laneRullers.end(),
				[](const TrendRuller& r1, const TrendRuller& r2)
				{
					return r1.timeStamp() < r2.timeStamp();
				});

			// Calc ruller timestamp text width
			//
			QRectF timeStampBoundRect;
			drawText(painter, " 00:00:00.000 ", QRectF(), drawParam, Qt::AlignCenter, &timeStampBoundRect);

			double rullerTextTop = laneRect.top() + (trendAreaRect.top() - laneRect.top()) / 2.0 - timeStampBoundRect.height() / 2.0;
			double rullerTextHeight = timeStampBoundRect.height();

			// Draw ruller line
			//
			painter->setClipRect(laneRect);

			double k = static_cast<double>(trendAreaRect.width()) / static_cast<double>(drawParam.duration());	// K is coefficient

			for (size_t i = 0; i < laneRullers.size(); i++)
			{
				//QRectF textBoundRect;
				const TrendRuller& r = laneRullers[i];

				if (r.timeStamp() < startLaneTime)
				{
					continue;
				}

				double x = trendAreaRect.left() + k * static_cast<double>(r.timeStamp().timeStamp - startLaneTime.timeStamp);
				x = static_cast<double>(static_cast<int>(x * dpiX)) / dpiX;		// Ajust x to look nice (not blurred)

				if (r.timeStamp() <= finishLaneTime)
				{
					painter->setPen(rullerPen);

					painter->drawLine(QPointF(x, trendAreaRect.top()),
									  QPointF(x, trendAreaRect.bottom()));

					if (r.timeStamp() == selectedRullerTime)
					{
						double xx = static_cast<double>(x * dpiX + 1) / dpiX;

						painter->drawLine(QPointF(xx, trendAreaRect.top()),
										  QPointF(xx, trendAreaRect.bottom()));
					}

					// Draw ruller timestamp
					//
					QString text = r.timeStamp().toDateTime().toString(" hh:mm:ss.zzz ");
					QRectF textRect(x - timeStampBoundRect.width() / 2.0,
									rullerTextTop,
									timeStampBoundRect.width(),
									rullerTextHeight);

					painter->fillRect(textRect, backgroundBrush);
					drawText(painter, text, textRect, drawParam, Qt::AlignCenter);
				}

				// Draw disctance between rullers
				//
				if (i > 0)
				{
					// There is a previouse ruller, draw distance to it
					//
					const TrendRuller& prevRuller = laneRullers[i - 1];

					double prevRullerX = trendAreaRect.left() + k * static_cast<double>(prevRuller.timeStamp().timeStamp - startLaneTime.timeStamp);
					if (prevRullerX < trendAreaRect.left())
					{
						prevRullerX = trendAreaRect.left();
					}
					else
					{
						prevRullerX += timeStampBoundRect.width() / 2.0;
					}

					x = static_cast<double>(static_cast<int>(x * dpiX)) / dpiX;		// Ajust x to look nice (not blurred)
					if (r.timeStamp() > finishLaneTime)
					{
						x = trendAreaRect.right();
					}
					else
					{
						x -= timeStampBoundRect.width() / 2.0;
					}

					double y = laneRect.top() + (trendAreaRect.top() - laneRect.top()) / 2.0;
					y = static_cast<double>(static_cast<int>(y * dpiY)) / dpiY;		// Ajust x to look nice (not blurred)

					if (prevRullerX < x)
					{
						painter->setPen(distancePen);
						painter->drawLine(QPointF(prevRullerX, y),
										  QPointF(x, y));
					}

					// Draw distance between rullers
					//
					qint64 rullersDistance = r.timeStamp().timeStamp - prevRuller.timeStamp().timeStamp;
					int msecs = rullersDistance % 1000_ms;
					int secs = (rullersDistance / 1_sec) % 60;
					int mins = (rullersDistance / 1_min) % 60;
					int hours = (rullersDistance / 1_hour) % 60;
					int days = (rullersDistance / 1_day) % 24;

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

					if (distanceTextBoundRect.width() + distanceTextBoundRect.height() / 2 < x - prevRullerX)
					{
						QRectF distanceTextRect;
						distanceTextRect.setLeft(prevRullerX + (x - prevRullerX) / 2.0 - distanceTextBoundRect.width() / 2.0);
						distanceTextRect.setTop(rullerTextTop);
						distanceTextRect.setWidth(distanceTextBoundRect.width());
						distanceTextRect.setHeight(distanceTextBoundRect.height());

						painter->fillRect(distanceTextRect, backgroundBrush);
						drawText(painter, distanceText, distanceTextRect, drawParam, Qt::AlignCenter);			// Draw distanmce bewtween rullers
					}
				}

				if (r.timeStamp() > finishLaneTime)
				{
					// Break here, not in the begining of the loop
					// We need to draw this (of lane) ruller, to draw distance to the perv ruller
					//
					break;
				}
			}
		}

		// Reset clipping
		//
		painter->setClipping(false);

		return;
	}

	void Trend::adjustPainter(QPainter* painter, int dpiX, int dpiY) const
	{
		assert(painter);

		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->setRenderHint(QPainter::TextAntialiasing, true);

		painter->resetTransform();

		painter->translate(0.5, 0.5);
		painter->scale(dpiX, dpiY);

		return;
	}

	void Trend::drawPolyline(QPainter* painter, const QVector<QPointF>& lines, const QRectF& rect) const
	{
		assert(painter);

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

	void Trend::calcSignalRects(QPainter* painter,
									const QRectF& insideRect,
									const TrendDrawParam& drawParam,
									std::vector<TrendSignalParam>* discretes,
									std::vector<TrendSignalParam>* analogs) const
	{
		assert(painter);
		assert(discretes);
		assert(analogs);

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

		if (drawParam.view() == TrendView::Separated &&
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

		if (drawParam.view() == TrendView::Overlapped  &&
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

		assert(drawParam.view() == TrendView::Separated ||
			   drawParam.view() == TrendView::Overlapped);

		return;
	}

	QRectF Trend::calcLaneRect(int laneIndex, const TrendDrawParam& drawParam)
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

	QRectF Trend::calcTrendArea(const QRectF& laneRect, const TrendDrawParam& drawParam)
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

		insideRect.setLeft(laneRect.left() + 6.0/8.0);
		insideRect.setTop(laneRect.top() + 1.0/5.0);
		insideRect.setWidth(laneRect.width() - insideRect.left() - 2.0/8.0);
		insideRect.setHeight(laneRect.height() - (insideRect.top() - laneRect.top()) - 0.3);

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

	QRect Trend::inchRectToPixelRect(const QRectF& rect, const TrendDrawParam& drawParam)
	{
		QRect result(static_cast<int>(rect.left() * drawParam.dpiX()),
					 static_cast<int>(rect.top() * drawParam.dpiY()),
					 static_cast<int>(rect.width() * drawParam.dpiX()),
					 static_cast<int>(rect.height() * drawParam.dpiY()));

		return result;
	}

	QRectF Trend::pixelRectToInchRect(const QRect& rect, const TrendDrawParam& drawParam)
	{
		QRectF result(static_cast<double>(rect.left()) / static_cast<double>(drawParam.dpiX()),
					  static_cast<double>(rect.top()) / static_cast<double>(drawParam.dpiY()),
					  static_cast<double>(rect.width()) / static_cast<double>(drawParam.dpiX()),
					  static_cast<double>(rect.height()) / static_cast<double>(drawParam.dpiY()));

		return result;
	}

	Trend::MouseOn Trend::mouseIsOver(QPoint mousePos, const TrendDrawParam& drawParam, int* outLaneIndex, TimeStamp* outTime, int* rullerIndex, QString* outSignalId) const
	{
		if (outLaneIndex == nullptr ||
			outTime == nullptr ||
			rullerIndex == nullptr ||
			outSignalId == nullptr)
		{
			assert(outLaneIndex);
			assert(outTime);
			assert(rullerIndex);
			assert(outSignalId);
			return Trend::MouseOn::Outside;
		}

		*outLaneIndex = -1;
		*rullerIndex = -1;
		*outTime = TimeStamp();
		outSignalId->clear();

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
					TimeStamp posTime = startLaneTime + timeOffset;

					*outTime = posTime;
					*outLaneIndex = laneIndex;

					// Check if pos OnRuller
					//
					qint64 deltaTime = static_cast<qint64>(coef * 1.0 / 32.0);
					const std::vector<TrendRuller>& rullers = rullerSet().rullers();
					int ri = 0;

					for (const TrendRuller& ruller : rullers)
					{
						if (posTime >= TimeStamp(ruller.timeStamp().timeStamp - deltaTime) &&
							posTime <= TimeStamp(ruller.timeStamp().timeStamp + deltaTime))
						{
							*rullerIndex = ri;
							return MouseOn::OnRuller;
						}

						ri++;
					}

					// Check if pos on signal description
					//
					for (const std::pair<QString, QRectF>& p : drawParam.signalDescriptionRect())
					{
						if (p.second.contains(pos) == true)
						{
							*outSignalId = p.first;
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

	double Trend::timeToScaledPixel(const TimeStamp& time, const QRectF& rect, const TimeStamp& startTime, qint64 duration)
	{
		if (duration == 0)
		{
			assert(duration != 0);
			duration = 1;
		}

		return rect.left() + (rect.width() / duration) * (time.timeStamp - startTime.timeStamp);
	}

	double Trend::valueToScaledPixel(double value, const QRectF& rect, double lowLimit, double highLimit)
	{
		double delta = fabs(highLimit - lowLimit);

		if (delta <= DBL_MIN)
		{
			assert(fabs(highLimit - lowLimit) > DBL_MIN);
			return 0;
		}

		return rect.bottom() - (rect.height() / delta) * (value - lowLimit);
	}


	void Trend::drawText(QPainter* painter, const QString& str, const QRectF& rect, const TrendDrawParam& drawParam, int flags, QRectF* boundingRect/* = nullptr*/)
	{
		if (painter == nullptr)
		{
			assert(painter);
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

		QFont f("Arial");

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

	TrendLib::TrendRullerSet& Trend::rullerSet()
	{
		return m_rullerSet;
	}

	const TrendLib::TrendRullerSet& Trend::rullerSet() const
	{
		return m_rullerSet;
	}

}
