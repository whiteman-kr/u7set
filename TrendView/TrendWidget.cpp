#include "TrendWidget.h"
#include <cstdlib>
#include <QPaintEngine>
#include <QPainter>
#include <QWidget>
#include <QMouseEvent>

namespace TrendLib
{
	RenderThread::RenderThread(Trend* trend, QObject* parent)
		: QThread(parent),
		m_trend(trend)
	{
		assert(m_trend);
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

			if (m_image.size() != drawParam.rect().size())
			{
				QSize pixelSize = drawParam.rect().size();
				qDebug() << "Create new trend image with size " << pixelSize;
				qDebug() << "dpiX = " << drawParam.dpiX();
				qDebug() << "dpiY = " << drawParam.dpiY();

				m_image = QImage(pixelSize, QImage::Format_RGB32);
			}

			// All drawing are done in inches
			//
			m_trend->draw(&m_image, drawParam);

			emit renderedImage(m_image);
		}
		while (isInterruptionRequested() == false);

		return;
	}


	TrendWidget::TrendWidget(QWidget* parent) :
		QWidget(parent),
		m_thread(&m_trend)
	{
		setMouseTracking(true);		// To enable mouseMoveEvent without pressed button

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

	void TrendWidget::mousePressEvent(QMouseEvent* event)
	{
		int laneIndex = -1;
		TimeStamp timeStamp;
		QPoint mousePos = event->pos();

		Trend::MouseOn mouseOn = mouseIsOver(mousePos, &laneIndex, &timeStamp);

		if (event->button() == Qt::LeftButton)
		{
			if (mouseOn == Trend::MouseOn::InsideTrendArea)
			{
				m_mouseScrollInitialTime = m_drawParam.startTimeStamp();
				m_mouseScrollInitialMousePos = event->pos();
				this->grabMouse();
			}

			return;
		}

		if (event->button() == Qt::RightButton)
		{
			return;
		}

		return;
	}

	void TrendWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
	{
		if (QWidget::mouseGrabber() == this)
		{
			releaseMouse();
		}

		return;
	}

	void TrendWidget::mouseMoveEvent(QMouseEvent* event)
	{
		if (QWidget::mouseGrabber() == nullptr)
		{
			int laneIndex = -1;
			TimeStamp timeStamp;
			QPoint mousePos = event->pos();

			Trend::MouseOn mouseOn = m_trend.mouseIsOver(mousePos, m_drawParam, &laneIndex, &timeStamp);

			QCursor cursor = Qt::ArrowCursor;

			switch (mouseOn)
			{
			case Trend::MouseOn::Outside:			cursor = Qt::ArrowCursor;		break;
			case Trend::MouseOn::OutsideTrendArea:	cursor = Qt::ArrowCursor;		break;
			case Trend::MouseOn::InsideTrendArea:	cursor = Qt::PointingHandCursor;break;
			default:
				assert(false);
			}

			setCursor(cursor);
			return;
		}

		if (QWidget::mouseGrabber() == this)
		{
			// Scroll area with a mouse mode
			//
			QRectF laneRect = m_trend.calcLaneRect(0, m_drawParam);
			QRectF trenAreaRect = m_trend.calcTrendArea(laneRect, m_drawParam);	// TrendArea in inches
			QRectF trendAreaRectPixels = Trend::inchRectToPixelRect(trenAreaRect, m_drawParam);

			double coefx = m_drawParam.duration() / trendAreaRectPixels.width();

			QPointF mouseOffset = m_mouseScrollInitialMousePos - event->pos();

			TimeStamp ts(m_mouseScrollInitialTime.timeStamp + static_cast<qint64>(mouseOffset.x() * coefx));
			m_drawParam.setStartTimeStamp(ts);

			updateWidget();

			emit startTimeChanged(ts);
		}

		return;
	}

	Trend::MouseOn TrendWidget::mouseIsOver(const QPoint& mousePos, int* outLaneIndex, TimeStamp* timeStamp)
	{
		return m_trend.mouseIsOver(mousePos, m_drawParam, outLaneIndex, timeStamp);
	}

	void TrendWidget::updatePixmap(const QImage& image)
	{
		m_pixmap = QPixmap::fromImage(image);

		update();
		return;
	}

	TrendLib::TrendSignalSet& TrendWidget::signalSet()
	{
		return m_trend.signalSet();
	}

	const TrendLib::TrendSignalSet& TrendWidget::signalSet() const
	{
		return m_trend.signalSet();
	}

	TrendLib::Trend& TrendWidget::trend()
	{
		return m_trend;
	}

	const TrendLib::Trend& TrendWidget::trend() const
	{
		return m_trend;
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

	TimeType TrendWidget::timeType() const
	{
		return m_drawParam.timeType();
	}

	void TrendWidget::setTimeType(TimeType value)
	{
		m_drawParam.setTimeType(value);
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
