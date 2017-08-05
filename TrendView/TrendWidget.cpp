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
		qRegisterMetaType<TrendLib::TrendDrawParam>("TrendDrawParam");

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

			emit renderedImage(m_image, drawParam);
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

		if (m_pixmap.isNull() == true)
		{
			painter.fillRect(rect(), Qt::white);
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

		// Draw rullers
		//
		trend().drawRullers(&painter, m_pixmapDrawParam);

		return;
	}

	void TrendWidget::resizeEvent(QResizeEvent*)
	{
		updateWidget();
	}

	void TrendWidget::mousePressEvent(QMouseEvent* event)
	{
		m_mouseAction = MouseAction::None;

		int laneIndex = -1;
		int rullerIndex = -1;
		TimeStamp timeStamp;

		Trend::MouseOn mouseOn = mouseIsOver(event->pos(), &laneIndex, &timeStamp, &rullerIndex);

		if (event->buttons().testFlag(Qt::LeftButton) == true)
		{
			if (mouseOn == Trend::MouseOn::OnRuller)
			{
				if (rullerIndex == -1)
				{
					assert(rullerIndex != -1);
					return;
				}

				m_rullerMoveRullerIndex = rullerIndex;
				m_rullerMoveInitialMousePos = event->pos();

				m_rullerMoveInitialTimeStamp = rullerSet().rullers().at(rullerIndex).timeStamp();

				m_mouseAction = MouseAction::MoveRuller;
				this->grabMouse();
			}

			if (mouseOn == Trend::MouseOn::InsideTrendArea)
			{
				m_mouseScrollInitialTime = m_drawParam.startTimeStamp();
				m_mouseScrollInitialMousePos = event->pos();

				m_mouseAction = MouseAction::Scroll;
				this->grabMouse();
			}

			return;
		}

		if (event->buttons().testFlag(Qt::RightButton) == true)
		{
			return;
		}

		return;
	}

	void TrendWidget::mouseReleaseEvent(QMouseEvent* event)
	{
		event->accept();

		if (m_mouseAction == MouseAction::MoveRuller)
		{
			// This will call slider update
			//
			if (dynamic_cast<QWidget*>(parent()) != nullptr)
			{
				dynamic_cast<QWidget*>(parent())->update();
			}
		}

		m_mouseAction = MouseAction::None;
		releaseMouse();

		if (event->buttons().testFlag(Qt::LeftButton) == false)
		{
			unsetCursor();
			mouseMoveEvent(event);		// To set cursor
		}
		else
		{
			unsetCursor();
		}

		return;
	}

	void TrendWidget::mouseMoveEvent(QMouseEvent* event)
	{
		if (m_mouseAction == MouseAction::None)
		{
			int laneIndex = -1;
			int rullerIndex = -1;
			TimeStamp timeStamp;

			Trend::MouseOn mouseOn = m_trend.mouseIsOver(event->pos(), m_drawParam, &laneIndex, &timeStamp, &rullerIndex);

			Qt::CursorShape newCursorShape = Qt::ArrowCursor;

			switch (mouseOn)
			{
			case Trend::MouseOn::Outside:			newCursorShape = Qt::ArrowCursor;		break;
			case Trend::MouseOn::OutsideTrendArea:	newCursorShape = Qt::ArrowCursor;		break;
			case Trend::MouseOn::InsideTrendArea:	newCursorShape = Qt::PointingHandCursor;break;
			case Trend::MouseOn::OnRuller:			newCursorShape = Qt::SplitHCursor;		break;
			default:
				assert(false);
			}

			m_pixmapDrawParam.setHightlightRullerIndex(rullerIndex);

			if (newCursorShape == Qt::ArrowCursor)
			{
				this->unsetCursor();
				update();
			}
			else
			{
				if (this->cursor().shape() != newCursorShape)
				{
					this->setCursor(newCursorShape);
					update();
				}
			}

			return;
		}

		if (QWidget::mouseGrabber() == this)
		{
			switch (m_mouseAction)
			{
			case MouseAction::None:
				assert(false);
				break;
			case MouseAction::Scroll:
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
				break;
			case MouseAction::MoveRuller:
				{
					assert(m_rullerMoveRullerIndex != -1);
					assert(m_rullerMoveRullerIndex >= 0 && m_rullerMoveRullerIndex < static_cast<int>(rullerSet().rullers().size()));

					int laneHeight = rect().height() / laneCount();
					int laneIndex = qBound<int>(0, event->pos().y() / laneHeight, laneCount() - 1);

					QRectF laneRect = m_trend.calcLaneRect(laneIndex, m_drawParam);
					QRectF trenAreaRect = m_trend.calcTrendArea(laneRect, m_drawParam);	// TrendArea in inches
					QRectF trendAreaRectPixels = Trend::inchRectToPixelRect(trenAreaRect, m_drawParam);

					qint64 laneStartTime = m_drawParam.startTimeStamp().timeStamp + m_drawParam.duration() * laneIndex;

					double coefx = m_drawParam.duration() / trendAreaRectPixels.width();

					int mouseOffset = event->pos().x() - trendAreaRectPixels.left();
					mouseOffset = qBound<int>(1, mouseOffset, trendAreaRectPixels.width());

					TimeStamp ts(laneStartTime + static_cast<qint64>(mouseOffset * coefx));

					TrendRuller& mutableRuller = rullerSet().rullers().at(m_rullerMoveRullerIndex);
					mutableRuller.setTimeStamp(ts);

					update();
				}
				break;
			default:
				assert(false);
				break;
			}
		}

		event->accept();
		return;
	}

	Trend::MouseOn TrendWidget::mouseIsOver(const QPoint& mousePos, int* outLaneIndex, TimeStamp* timeStamp, int* rullerIndex)
	{
		return m_trend.mouseIsOver(mousePos, m_drawParam, outLaneIndex, timeStamp, rullerIndex);
	}

	void TrendWidget::resetRullerHighlight()
	{
		m_pixmapDrawParam.resetHightlightRullerIndex();
	}


	void TrendWidget::updatePixmap(const QImage& image, TrendDrawParam drawParam)
	{
		m_pixmap = QPixmap::fromImage(image);
		m_pixmapDrawParam = drawParam;

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

	TrendLib::TrendRullerSet& TrendWidget::rullerSet()
	{
		return m_trend.rullerSet();
	}

	const TrendLib::TrendRullerSet& TrendWidget::rullerSet() const
	{
		return m_trend.rullerSet();
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
