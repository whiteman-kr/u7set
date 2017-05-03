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

			//////// All drawing are done in inches
			///////
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

			for (int laneIndex = 0; laneIndex < drawParam.laneCount(); laneIndex++)
			{
				QRectF laneRect;

				laneRect.setLeft(laneMargin);
				laneRect.setRight(inchSize.width() - laneMargin * 2.0);

				laneRect.setTop(laneMargin + static_cast<double>(laneIndex) * (laneHeight + laneMargin));
				laneRect.setHeight(laneHeight);

				drawLane(&painter, laneRect, drawParam);
			}

			// --
			//
//			painter.setBrush(Qt::red);
//			QPen ppp(Qt::green, 0);
//			painter.setPen(ppp);
//			//painter.setPen(Qt::PenStyle::NoPen);
//			painter.drawRect(QRectF(1, 1, 2, 1));

//			double x = 0;
//			double y = 0;
//			double width = 150;
//			double height = 50;

//			double dx = 20;
//			double dy = 20;

//			for (int i = 0; i < 1000; i++)
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

//				x += dx;
//				y += dy;

//				if (x >= size.width() - width ||
//					x < 0)
//				{
//					dx *= -1;
//				}

//				if (y >= size.height() - height ||
//					y < 0)
//				{
//					dy *= -1;
//				}

//				QFont f("Arial", 30);
//				f.setStyleStrategy(QFont::StyleStrategy(QFont::PreferAntialias | QFont::OpenGLCompatible));

//				painter.setPen(Qt::blue);
//				painter.setFont(f);
//				painter.drawText(QRectF(x, y, width, height), Qt::AlignCenter, "Qt");
//			}

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
