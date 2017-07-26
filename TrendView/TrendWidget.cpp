#include "TrendWidget.h"
#include <cstdlib>
#include <QPaintEngine>
#include <QPainter>

namespace TrendLib
{
	RenderThread::RenderThread(TrendSignalSet* signalSet, QObject* parent)
		: QThread(parent),
		m_signalSet(signalSet)
	{
		assert(m_signalSet);
	}

	RenderThread::~RenderThread()
	{
		requestInterruption();

		bool ok = wait(5000);
		if (ok == false)
		{
			qDebug() << "TREND RENDER THREAD IS NOT FINISHED, IT WILL BE TERMINATED!!!";
			terminate();
		}

		return;
	}

	void RenderThread::render(const TrendDrawParam& drawParam)
	{
		QMutexLocker locker(&m_mutex);

		m_drawParam = drawParam;
		m_newJob = true;

		if (isRunning() == false)
		{
			start(QThread::NormalPriority);
		}

		return;
	}

	void RenderThread::run()
	{
		do
		{
			QThread::msleep(5);

			if (m_newJob == false)
			{
				continue;
			}

			// Start new job
			//
			m_mutex.lock();
			TrendDrawParam drawParam = m_drawParam;
			m_mutex.unlock();

			// Set m_newJob to false, so it can be raised again while current drawing in progress
			//
			m_newJob = false;

			// All drawing are done in inches
			//
			QTime timeMeasures;
			timeMeasures.start();

			QSize pixelSize = drawParam.rect().size();

			QSizeF inchSize = drawParam.rect().size();
			inchSize.setWidth(inchSize.width() / drawParam.dpiX());
			inchSize.setHeight(inchSize.height() / drawParam.dpiY());

			if (m_image.size() != pixelSize)
			{
				qDebug() << "Create new trend image with size " << pixelSize;
				qDebug() << "dpiX = " << drawParam.dpiX();
				qDebug() << "dpiY = " << drawParam.dpiY();

				m_image = QImage(pixelSize, QImage::Format_RGB32);
			}

			m_image.fill(Qt::white);

			QPainter painter(&m_image);
			painter.setRenderHint(QPainter::Antialiasing, true);
			painter.setRenderHint(QPainter::TextAntialiasing, true);

			//--
			//
			painter.resetTransform();

			painter.translate(0.5, 0.5);
			painter.scale(drawParam.dpiX(), drawParam.dpiY());

			// --
			//
			double laneMargin = 1.0 / 32.0;		// 1/16 inch
			double laneHeight = (inchSize.height() - laneMargin) / static_cast<double>(drawParam.laneCount()) - laneMargin;

			QDateTime startTime = drawParam.startTime();

			for (int laneIndex = 0; laneIndex < drawParam.laneCount(); laneIndex++)
			{
				if (isInterruptionRequested() == true)
				{
					break;
				}

				QRectF laneRect;

				laneRect.setLeft(laneMargin);
				laneRect.setRight(inchSize.width() - laneMargin * 2.0);

				laneRect.setTop(laneMargin + static_cast<double>(laneIndex) * (laneHeight + laneMargin));
				laneRect.setHeight(laneHeight);

				drawParam.setStartTime(startTime);

				drawLane(&painter, laneRect, drawParam);

				startTime = startTime.addMSecs(drawParam.duration());
			}

			static int DrawImageCounter = 0;
			qDebug() << "DrawImageCounter " << ++DrawImageCounter << ", trend draw time: " << timeMeasures.elapsed() << " ms";

			emit renderedImage(m_image);
		}
		while (isInterruptionRequested() == false);

		return;
	}

	void RenderThread::drawLane(QPainter* painter, const QRectF& rect, const TrendDrawParam& drawParam)
	{
		painter->setBrush(drawParam.backgroundColor());
		painter->setPen(Qt::PenStyle::NoPen);
		painter->drawRect(rect);

		double dpiX = drawParam.dpiX();
		double dpiY = drawParam.dpiY();

		// Calc InsideRect
		// +-------------------------------+
		// |  +---------------------------+|
		// |  |      insideRect           ||
		// |  +---------------------------+|
		// +-------------------------------+
		//
		QRectF insideRect;

		insideRect.setLeft(rect.left() + 6.0/8.0);
		insideRect.setTop(rect.top() + 1.0/8.0);
		insideRect.setWidth(rect.width() - insideRect.left() - 2.0/8.0);
		insideRect.setHeight(rect.height() - (insideRect.top() - rect.top()) - 2.0/8.0);

		// Ajust inside rect to dpiX, so it will look pretty while drawing it with cosmetic pen
		//
		insideRect.setLeft(static_cast<double>(static_cast<int>(insideRect.left() * dpiX)) / dpiX);
		insideRect.setTop(static_cast<double>(static_cast<int>(insideRect.top() * dpiY)) / dpiY);
		insideRect.setWidth(static_cast<double>(static_cast<int>(insideRect.width() * dpiX)) / dpiX);
		insideRect.setHeight(static_cast<double>(static_cast<int>(insideRect.height() * dpiY)) / dpiY);

		// Draw trend in separate mode
		//
		assert(m_signalSet);

		std::vector<TrendSignalParam> discreteSignals = m_signalSet->discreteSignals();
		std::vector<TrendSignalParam> analogSignals = m_signalSet->analogSignals();

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

	void RenderThread::drawTimeGrid(QPainter* painter, const QRectF& rect, const QRectF& insideRect, const TrendDrawParam& drawParam)
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

			double x = timeToPixel(ct, insideRect, startTimeStamp, duration);

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
			double x = timeToPixel(ct, insideRect, startTimeStamp, duration);

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

	void RenderThread::drawSignal(QPainter* painter, const TrendSignalParam& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor)
	{
		assert(painter);

		painter->fillRect(rect, backColor);

		painter->setPen(signal.color());

		QString signalText = QString("  %1 - %2").arg(signal.signalId()).arg(signal.caption());
		drawText(painter, signalText, rect, drawParam, Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine | Qt::TextDontClip);

		if (signal.isDiscrete() == true)
		{
			drawDiscrete(painter, signal, rect, drawParam, backColor);
		}
		else
		{
			drawAnalog(painter, signal, rect, drawParam, backColor);
		}

		return;
	}

	void RenderThread::drawDiscrete(QPainter* painter, const TrendSignalParam& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor)
	{
		assert(painter);
		assert(signal.isDiscrete() == true);

		// Draw units (0, 1) on the left side of rect
		//
		painter->setPen(signal.color());

		QRectF textBoundRect;
		drawText(painter, "0 ", QRectF(rect.left(), rect.bottom(), 0, 0), drawParam, Qt::AlignRight | Qt::AlignBottom | Qt::TextDontClip, &textBoundRect);
//		drawText(painter, "1 ", QRectF(rect.left(), rect.top() + textBoundRect.height(), 0, 0), drawParam, Qt::AlignRight | Qt::AlignVCenter | Qt::TextDontClip);

		// Draw trend
		//
		std::list<std::shared_ptr<OneHourData>> outData;

		QDateTime startTime = drawParam.startTime();
		QDateTime finishTime = TimeStamp(drawParam.startTimeStamp().timeStamp + drawParam.duration()).toDateTime();

		bool requestOk = m_signalSet->getTrendData(signal.appSignalId(), startTime, finishTime, &outData);

		if (requestOk == false)
		{
			return;
		}

		QPen linePen;
		linePen.setCosmetic(true);
		linePen.setColor(signal.color());
		painter->setPen(linePen);

		QVector<QPointF> lines;
		//TrendStateRecord lastRecord;

		TimeStamp startTimeStamp = drawParam.startTimeStamp();
		qint64 duration = drawParam.duration();

		double yPos0 = rect.bottom() - textBoundRect.height() / 2.0;
		double yPos1 = rect.top() + textBoundRect.height() / 2.0 ;

		double lastX = 0;
		double lastY = 0;

		for (std::shared_ptr<OneHourData> hour : outData)
		{
			const std::vector<TrendStateRecord>& data = hour->data;

			for (const TrendStateRecord& record : data)
			{
				for (const TrendStateItem& state : record.states)
				{
					TimeStamp ct = state.local;

					//qDebug() << ct.toDateTime() << ", value " << state.value << ", flags" << state.flags;

					if (state.isValid() == false &&
						lines.isEmpty() == false)
					{
						QPolygonF pf(lines);
						painter->drawPolyline(lines);
						lines.clear();
						continue;
					}

					double x = timeToPixel(ct, rect, startTimeStamp, duration);
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

		return;
	}

	void RenderThread::drawAnalog(QPainter* painter, const TrendSignalParam& signal, const QRectF& rect, const TrendDrawParam& drawParam, QColor backColor)
	{
		assert(painter);
		assert(signal.isAnalog() == true);

		// Draw units (0, 1) on the left side of rect
		//
//		painter->setPen(signal.color());

//		QRectF textBoundRect;
//		drawText(painter, "0 ", QRectF(rect.left(), rect.bottom(), 0, 0), drawParam, Qt::AlignRight | Qt::AlignBottom | Qt::TextDontClip, &textBoundRect);
//		drawText(painter, "1 ", QRectF(rect.left(), rect.top() + textBoundRect.height(), 0, 0), drawParam, Qt::AlignRight | Qt::AlignVCenter | Qt::TextDontClip);

		// Draw trend
		//
		std::list<std::shared_ptr<OneHourData>> outData;

		QDateTime startTime = drawParam.startTime();
		QDateTime finishTime = TimeStamp(drawParam.startTimeStamp().timeStamp + drawParam.duration()).toDateTime();

		bool requestOk = m_signalSet->getTrendData(signal.appSignalId(), startTime, finishTime, &outData);

		return;
	}

	double RenderThread::timeToPixel(const TimeStamp& time, const QRectF& rect, const TimeStamp& startTime, qint64 duration)
	{
		if (duration == 0)
		{
			assert(duration != 0);
			duration = 1;
		}

		return rect.left() + (rect.width() / duration) * (time.timeStamp - startTime.timeStamp);
	}

	void RenderThread::drawText(QPainter* painter, const QString& str, const QRectF& rect, const TrendDrawParam& drawParam, int flags, QRectF* boundingRect/* = nullptr*/)
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

	TrendWidget::TrendWidget(TrendSignalSet* signalSet, QWidget* parent) :
		QWidget(parent),
		m_thread(signalSet),
		m_signalSet(signalSet)
	{
		assert(m_signalSet);

		connect(&m_thread, &RenderThread::renderedImage, this, &TrendWidget::updatePixmap);
	}

	TrendWidget::~TrendWidget()
	{
	}

	void TrendWidget::updateWidget()
	{
		m_drawParam.setRect(rect());
		m_drawParam.setDpi(physicalDpiX(), physicalDpiY());

		m_thread.render(m_drawParam);
	}

	void TrendWidget::paintEvent(QPaintEvent*)
	{
		QPainter painter(this);
		painter.fillRect(rect(), Qt::white);

		if (m_pixmap.isNull() == true)
		{
			painter.setPen(Qt::black);
			painter.drawText(rect(), Qt::AlignCenter, tr("Rendering initial image, please wait..."));
			return;
		}

		if (m_pixmap.size() != rect().size())
		{
			// New pixmap is not ready yet, scale the current one
			//
			painter.drawPixmap(rect(), m_pixmap, m_pixmap.rect());
			return;
		}

		painter.drawPixmap(0, 0, m_pixmap);

		return;
	}

	void TrendWidget::resizeEvent(QResizeEvent*)
	{
		updateWidget();
	}

	void TrendWidget::updatePixmap(const QImage& image)
	{
		m_pixmap = QPixmap::fromImage(image);

		update();
		return;
	}

	TrendView TrendWidget::view() const
	{
		return m_drawParam.view();
	}

	void TrendWidget::setView(TrendView value)
	{
		m_drawParam.setView(value);
		return;
	}

	int TrendWidget::laneCount() const
	{
		return m_drawParam.laneCount();
	}

	void TrendWidget::setLaneCount(int value)
	{
		m_drawParam.setLaneCount(value);
	}

	void TrendWidget::setStartTime(const TimeStamp& startTime)
	{
		m_drawParam.setStartTimeStamp(startTime);
	}

	void TrendWidget::setDuration(qint64 interval)
	{
		m_drawParam.setDuration(interval);
	}

}
