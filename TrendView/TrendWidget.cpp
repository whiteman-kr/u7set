#include "TrendWidget.h"
#include <cstdlib>
#include <QPaintEngine>
#include <QPainter>

namespace TrendLib
{
	RenderThread::RenderThread(QObject* parent)
		: QThread(parent)
	{
	}

	RenderThread::~RenderThread()
	{
		m_mutex.lock();
		m_abort = true;
		m_condition.wakeOne();
		m_mutex.unlock();

		wait(5000);
	}

	void RenderThread::render(const TrendDrawParam& drawParam)
	{
		QMutexLocker locker(&m_mutex);

		this->m_drawParam = drawParam;

		if (isRunning() == false)
		{
			start(QThread::NormalPriority);
		}
		else
		{
			m_restart = true;
			m_condition.wakeOne();
		}

		return;
	}

	void RenderThread::run()
	{
		do
		{
			m_mutex.lock();
			TrendDrawParam drawParam = m_drawParam;
			m_mutex.unlock();

			// All drawing are done in inches
			//
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

			//painter.drawLine(0, 0, pixelSize.width(), pixelSize.height());
			//painter.drawLine(pixelSize.width(), 0, 0, pixelSize.height());

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
				QRectF laneRect;

				laneRect.setLeft(laneMargin);
				laneRect.setRight(inchSize.width() - laneMargin * 2.0);

				laneRect.setTop(laneMargin + static_cast<double>(laneIndex) * (laneHeight + laneMargin));
				laneRect.setHeight(laneHeight);

				drawParam.setStartTime(startTime);

				drawLane(&painter, laneRect, drawParam);

				startTime.addMSecs(drawParam.duration());
			}

			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//
//			painter.setBrush(Qt::red);
//			QPen ppp(Qt::green, 0);
//			painter.setPen(ppp);
//			//painter.setPen(Qt::PenStyle::NoPen);
//			painter.drawRect(QRectF(1, 1, 2, 1));

//			double x = 0;
//			double y = 0;
//			double width = 1;
//			double height = 2;

//			double dx = 0.1;
//			double dy = 0.1;

//			QTime t;
//			t.start();

//			QPen pppp;
//			pppp.setCosmetic(true);
//			pppp.setColor(Qt::blue);
//			painter.setPen(pppp);
//			for (int i = 0; i < 10000; i++)
//			{
//				if (m_restart == true)
//				{
//					//emit renderedImage(m_image);
//					break;
//				}

//				if (m_abort == true)
//				{
//					return;
//				}

//				painter.drawLine(QPointF((double)i * 0.001, 0), QPointF((double)i * 0.002, inchSize.height()));

//				x += dx;
//				y += dy;

//				if (x >= inchSize.width() - width ||
//					x < 0)
//				{
//					dx *= -1;
//				}

//				if (y >= inchSize.height() - height ||
//					y < 0)
//				{
//					dy *= -1;
//				}

//				QFont f("Arial", 0.75);
//				f.setStyleStrategy(QFont::StyleStrategy(QFont::PreferAntialias | QFont::OpenGLCompatible));

//				painter.setPen(Qt::blue);
//				painter.setFont(f);
//				painter.drawText(QRectF(x, y, width, height), Qt::AlignCenter, "Qt");
//			}
//			qDebug() << "Elapsed " << t.elapsed();
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//

			if (m_restart != true)
			{
				emit renderedImage(m_image);
			}

			m_mutex.lock();
			if (m_restart == false)
			{
				m_condition.wait(&m_mutex);
			}
			m_restart = false;
			m_mutex.unlock();
		}
		while (m_abort == false);

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

		// Ajust insede rect to dpiX, so it will look pretty while drawing it with cosmetic pen
		//
		insideRect.setLeft(static_cast<double>(static_cast<int>(insideRect.left() * dpiX)) / dpiX);
		insideRect.setTop(static_cast<double>(static_cast<int>(insideRect.top() * dpiY)) / dpiY);
		insideRect.setWidth(static_cast<double>(static_cast<int>(insideRect.width() * dpiX)) / dpiX);
		insideRect.setHeight(static_cast<double>(static_cast<int>(insideRect.height() * dpiY)) / dpiY);

		// Draw insideRect
		//
		QPen insideRectPen;
		insideRectPen.setCosmetic(true);
		insideRectPen.setColor(Qt::darkGray);
		painter->setPen(insideRectPen);

		painter->drawRect(insideRect);

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

		// calc time grid positions
		//
		struct PosTimePair
		{
			double x;
			TimeStamp timeStamp;
		};

		std::vector<PosTimePair> timeGridPos;

		int timeGridCount = static_cast<int>(insideRect.width() / inchGridInterval);
		timeGridPos.reserve(timeGridCount + 1);

		for (int i = 0; i < timeGridCount + 1; i++)
		{
			TimeStamp ct = TimeStamp{drawParam.startTimeStamp().timeStamp + i * timeGridInterval};
			double x = timeToPixel(ct, insideRect, startTimeStamp, duration);

			// Make sure that x is proper alligned for nice look of cosmetic pen
			//
			x = static_cast<double>(static_cast<int>(x * dpiX)) / dpiX;

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

			QDate date = p.timeStamp.toDateTime().date();

			if (lastDate != date)
			{
				lastDate = date;

				double x = static_cast<double>(static_cast<int>(p.x * dpiX) + 1) / dpiX;

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

		// --
		//

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

		painter->drawText(rc, flags, str, boundingRect);

		painter->restore();
		return;
	}

	TrendWidget::TrendWidget(TrendSignalSet* signalSet, QWidget* parent) :
		QWidget(parent),
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

}
