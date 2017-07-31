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

		// --
		//
		image->fill(Qt::white);

		QPainter painter(image);
		adjustPainter(&painter, drawParam.dpiX(), drawParam.dpiY());

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

			drawLane(&painter, laneRect, laneDrawParam);			// Draw whole lane

			startTime = startTime.addMSecs(laneDrawParam.duration());
		}

		// --
		//
		qDebug() << "Trend draw time: " << timeMeasures.elapsed() << " ms";
		return;
	}

	void Trend::drawLane(QPainter* painter, const QRectF& rect, const TrendDrawParam& drawParam) const
	{
		painter->setBrush(drawParam.backgroundColor());
		painter->setPen(Qt::PenStyle::NoPen);
		painter->drawRect(rect);

		// Calc InsideRect
		// +-------------------------------+
		// |  +---------------------------+|
		// |  |   insideRect (trendArea)  ||
		// |  +---------------------------+|
		// +-------------------------------+
		//
		QRectF insideRect = calcTrendArea(rect, drawParam);

		// Draw trend in separate mode
		//
		std::vector<TrendSignalParam> discreteSignals = signalSet().discreteSignals();
		std::vector<TrendSignalParam> analogSignals = signalSet().analogSignals();

		if (drawParam.view() == TrendView::Separated)
		{
			const double discreteSignalHeight = 5.0 / 8.0;		// of inch

			double y = insideRect.top();

			QColor signalBackColor = drawParam.backgroundColor();

			for (const TrendSignalParam& s : discreteSignals)
			{
				QRectF signalRect = {insideRect.left(), y, insideRect.width(), discreteSignalHeight};
				y += discreteSignalHeight;

				if (signalRect.top() >= insideRect.bottom())
				{
					break;
				}

				if (signalRect.bottom() > insideRect.bottom())
				{
					signalRect.setBottom(insideRect.bottom());
				}

				signalBackColor = (signalBackColor == drawParam.laneBackgroundColor()) ? drawParam.backgroundColor() : drawParam.laneBackgroundColor();

				drawSignal(painter, s, signalRect, drawParam, signalBackColor);
			}

			const double analogSignalsSignalHeight = qMax((insideRect.bottom() - y) / analogSignals.size(), discreteSignalHeight);

			for (const TrendSignalParam& s : analogSignals)
			{
				QRectF signalRect = {insideRect.left(), y, insideRect.width(), analogSignalsSignalHeight};
				y += analogSignalsSignalHeight;

				if (signalRect.top() >= insideRect.bottom())
				{
					break;
				}

				if (signalRect.bottom() > insideRect.bottom())
				{
					signalRect.setBottom(insideRect.bottom());
				}

				signalBackColor = (signalBackColor == drawParam.laneBackgroundColor()) ? drawParam.backgroundColor() : drawParam.laneBackgroundColor();

				drawSignal(painter, s, signalRect, drawParam, signalBackColor);
			}
		}

		if (drawParam.view() == TrendView::Overlapped)
		{

		}

		// Draw insideRect
		//
		QPen insideRectPen;
		insideRectPen.setCosmetic(true);
		insideRectPen.setColor(Qt::darkGray);
		painter->setPen(insideRectPen);

		painter->setBrush(Qt::BrushStyle::NoBrush);

		painter->drawRect(insideRect);

		// Draw Time grid
		//
		drawTimeGrid(painter, rect, insideRect, drawParam);

		// --
		//

		return;
	}

	void Trend::drawTimeGrid(QPainter* painter, const QRectF& rect, const QRectF& insideRect, const TrendDrawParam& drawParam) const
	{
		double dpiX = drawParam.dpiX();

		// Calc time grid
		//
		static const std::array<qint64, 25> possibleTimeGridIntervals = {100_ms, 200_ms, 250_ms, 500_ms,
																		 1_sec, 2_sec, 5_sec, 10_sec, 15_sec, 20_sec, 30_sec,
																		 1_min, 90_sec, 2_min, 5_min, 10_min, 15_min, 20_min, 30_min,
																		 1_hour, 2_hours, 3_hours, 6_hours, 12_hours, 24_hours};

		double minTimeInterval = 3.0/4.0;	// 3/4 in -- minimum inches interval

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
		QPen timeGridPen(Qt::PenStyle::DashLine);
		timeGridPen.setCosmetic(true);
		timeGridPen.setColor(Qt::darkGray);
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

		painter->setPen(Qt::black);
		QString lastDateText;
		for (const PosTimePair& p : timeGridPos)
		{
			QString timeText = p.timeStamp.toDateTime().toString(timeGridInterval < 1_sec ? "hh:mm:ss.zzz" : "hh:mm:ss");
			QString dateText = p.timeStamp.toDateTime().toString("dd.MM.yyyy");

			QRectF timeTextRect(p.x - 2.0, insideRect.bottom(), 4.0, (rect.bottom() - insideRect.bottom()) / 2.0);
			QRectF dateTextRect(p.x - 2.0, timeTextRect.bottom(), 4.0, (rect.bottom() - insideRect.bottom()) / 2.0);

			drawText(painter, timeText, timeTextRect, drawParam, Qt::AlignCenter);

			if (lastDateText != dateText)
			{
				lastDateText = dateText;
				drawText(painter, dateText, dateTextRect, drawParam, Qt::AlignCenter);
			}
		}
	}

	void Trend::drawSignal(QPainter* painter, const TrendSignalParam& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor) const
	{
		assert(painter);

		painter->fillRect(rect, backColor);
		// --
		//
		painter->setPen(signal.color());

		painter->setClipRect(rect);		// Set clipo region, as SignalID and caption can fo out of drawArea

		QString signalText = QString("  %1 - %2").arg(signal.signalId()).arg(signal.caption());
		drawText(painter, signalText, rect, drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine | Qt::TextDontClip);

		painter->setClipping(false);	// Restore clip region

		// Get signal data
		//
		QDateTime startTime = drawParam.startTime();
		QDateTime finishTime = TimeStamp(drawParam.startTimeStamp().timeStamp + drawParam.duration()).toDateTime();

		std::list<std::shared_ptr<OneHourData>> signalData;

		bool requestResult = signalSet().getTrendData(signal.appSignalId(), startTime, finishTime, drawParam.timeType(), &signalData);
		if (requestResult == false)
		{
			signalData.clear();
		}

		// Draw signal trend
		//
		if (signal.isDiscrete() == true)
		{
			drawDiscrete(painter, signal, rect, drawParam, backColor, signalData);
		}
		else
		{
			drawAnalog(painter, signal, rect, drawParam, backColor, signalData);
		}

		return;
	}

	void Trend::drawDiscrete(QPainter* painter, const TrendSignalParam& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor /*backColor*/, const std::list<std::shared_ptr<OneHourData> >& signalData) const
	{
		assert(painter);
		assert(signal.isDiscrete() == true);

		// Draw units (0, 1) on the left side of rect
		//
		painter->setPen(signal.color());

		QRectF textBoundRect;
		drawText(painter, "0 ", QRectF(rect.left(), rect.bottom(), 0, 0), drawParam, Qt::AlignRight | Qt::AlignBottom | Qt::TextDontClip, &textBoundRect);	// Get bound rect, for understaning text height
		drawText(painter, "1 ", QRectF(rect.left(), rect.top() + textBoundRect.height(), 0, 0), drawParam, Qt::AlignRight | Qt::AlignVCenter | Qt::TextDontClip);

		// Set clip region
		//
		painter->setClipRect(rect);

		// Draw trend
		//
		TimeType timeType = drawParam.timeType();

		QPen linePen;
		linePen.setCosmetic(true);
		linePen.setColor(signal.color());
		painter->setPen(linePen);

		QVector<QPointF> lines;

		TimeStamp startTimeStamp = drawParam.startTimeStamp();
		qint64 duration = drawParam.duration();

		double yPos0 = rect.bottom() - textBoundRect.height() / 2.0;
		double yPos1 = rect.top() + textBoundRect.height() * 0.8 ;

		double lastX = 0;
		double lastY = 0;

		for (std::shared_ptr<OneHourData> hour : signalData)
		{
			const std::vector<TrendStateRecord>& data = hour->data;

			for (const TrendStateRecord& record : data)
			{
				for (const TrendStateItem& state : record.states)
				{
					TimeStamp ct;

					switch (timeType)
					{
					case TimeType::Local:	ct = state.local;	break;
					case TimeType::System:	ct = state.system;	break;
					case TimeType::Plant:	ct = state.plant;	break;
					default:
						assert(false);
						ct = state.local;
						break;
					}

					// Break line if it is not valid point
					//
					if (state.isValid() == false &&
						lines.isEmpty() == false)
					{
						painter->drawPolyline(lines);
						lines.clear();
						continue;
					}

					double x = timeToScaledPixel(ct, rect, startTimeStamp, duration);
					double y = (state.value == 0) ? yPos0 : yPos1;

					if (lines.isEmpty() == true)
					{
						lines.push_back(QPointF(x, y));

						lastX = x;
						lastY = y;
					}
					else
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
			}
		}

		if (lines.size() >= 2)
		{
			QPolygonF pf(lines);
			painter->drawPolyline(lines);
			lines.clear();
		}

		// Reset clipping
		//
		painter->setClipping(false);

		return;
	}

	void Trend::drawAnalog(QPainter* painter, const TrendSignalParam& signal, const QRectF& /*rect*/, const TrendDrawParam& drawParam, QColor /*backColor*/, const std::list<std::shared_ptr<OneHourData>>& /*signalData*/) const
	{
		assert(painter);
		assert(signal.isAnalog() == true);

		return;
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

		QPen rullerPen(Qt::PenStyle::DashLine);
		rullerPen.setCosmetic(true);
		rullerPen.setColor(qRgb(0x00, 0x00, 0xC0));
		painter->setPen(rullerPen);


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

			std::vector<TrendRuller> laneRullers = rullerSet().getRullers(startLaneTime, finishLaneTime);

			for (const TrendRuller& r : laneRullers)
			{
				double k = static_cast<double>(trendAreaRect.width()) / static_cast<double>(drawParam.duration());
				double x = trendAreaRect.left() + k * static_cast<double>(r.timeStamp().timeStamp - startLaneTime.timeStamp);
				x = static_cast<double>(static_cast<int>(x * dpiX)) / dpiX;		// Ajust x to look nice (not blurred)

				painter->drawLine(QPointF(x, trendAreaRect.top()),
								  QPointF(x, trendAreaRect.bottom()));

				if (r.timeStamp() == selectedRullerTime)
				{
					x = static_cast<double>(x * dpiX + 1) / dpiX;

					painter->drawLine(QPointF(x, trendAreaRect.top()),
									  QPointF(x, trendAreaRect.bottom()));
				}
			}
		}

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
		insideRect.setTop(laneRect.top() + 1.0/8.0);
		insideRect.setWidth(laneRect.width() - insideRect.left() - 2.0/8.0);
		insideRect.setHeight(laneRect.height() - (insideRect.top() - laneRect.top()) - 2.0/8.0);

		// Ajust inside rect to dpiX, so it will look pretty while drawing it with cosmetic pen
		//
		insideRect.setLeft(static_cast<double>(static_cast<int>(insideRect.left() * dpiX)) / dpiX);
		insideRect.setTop(static_cast<double>(static_cast<int>(insideRect.top() * dpiY)) / dpiY);
		insideRect.setWidth(static_cast<double>(static_cast<int>(insideRect.width() * dpiX)) / dpiX);
		insideRect.setHeight(static_cast<double>(static_cast<int>(insideRect.height() * dpiY)) / dpiY);

		return insideRect;
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

	Trend::MouseOn Trend::mouseIsOver(QPoint mousePos, const TrendDrawParam& drawParam, int* outLaneIndex, TimeStamp* outTime, int* rullerIndex) const
	{
		if (outLaneIndex == nullptr ||
			outTime == nullptr ||
			rullerIndex == nullptr)
		{
			assert(outLaneIndex);
			assert(outTime);
			assert(rullerIndex);
			return Trend::MouseOn::Outside;
		}

		*outLaneIndex = -1;
		*rullerIndex = -1;
		*outTime = TimeStamp();

		QRect rect = drawParam.rect();

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

	TimeStamp Trend::pixelToTime(int /*pos*/, const TrendDrawParam& /*drawParam*/)
	{
		// To do
		//
		assert(false);
		return TimeStamp();
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
