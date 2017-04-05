#include "TrendWidget.h"
#include <cstdlib>

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

	void RenderThread::render(double centerX, double centerY, double scaleFactor, QSize resultSize)
	{
		QMutexLocker locker(&m_mutex);

		this->m_centerX = centerX;
		this->m_centerY = centerY;
		this->m_scaleFactor = scaleFactor;
		this->m_resultSize = resultSize;

		if (isRunning() == false)
		{
			start(LowPriority);
		} else
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
				QSize resultSize = this->m_resultSize;
				double scaleFactor = this->m_scaleFactor;
				//double centerX = this->m_centerX;
				//double centerY = this->m_centerY;
			m_mutex.unlock();

			QImage image(resultSize, QImage::Format_RGB32);

			QPainter painter(&image);

			double x = 0;
			double y = 0;
			double width = 150;
			double height = 50;

			double dx = 20;
			double dy = 20;

			for (int i = 0; i < 1000; i++)
			{
				if (m_restart == true)
				{
					break;
				}

				if (m_abort == true)
				{
					return;
				}

				x += dx;
				y += dy;

				if (x >= resultSize.width() - width ||
					x < 0)
				{
					dx *= -1;
				}

				if (y >= resultSize.height() - height ||
					y < 0)
				{
					dy *= -1;
				}

				QFont f("Arial", 30);
				f.setStyleStrategy(QFont::StyleStrategy(QFont::PreferAntialias | QFont::OpenGLCompatible));

				painter.setPen(Qt::blue);
				painter.setFont(f);
				painter.drawText(QRectF(x, y, width, height), Qt::AlignCenter, "Qt");
			}

			if (m_restart != true)
			{
				emit renderedImage(image, scaleFactor);
			}

			m_mutex.lock();
			if (m_restart == false)
			{
				m_condition.wait(&m_mutex);
			}
			m_restart = false;
			m_mutex.unlock();
		}
		while (true);

		return;
	}

	TrendWidget::TrendWidget(QWidget* parent) :
		QWidget(parent)
	{

		connect(&m_thread, &RenderThread::renderedImage, this, &TrendWidget::updatePixmap);
	}

	TrendWidget::~TrendWidget()
	{
	}

	void TrendWidget::updateWidget()
	{
		m_thread.render(0, 0, 0, rect().size());
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

		if (m_pixmap.size() !=  rect().size())
		{
			// New pixmap is not ready yet, scale the current one
			//
			painter.drawPixmap(rect(), m_pixmap);
			return;
		}

		painter.drawPixmap(0, 0, m_pixmap);

		return;
	}

	void TrendWidget::resizeEvent(QResizeEvent*)
	{
		updateWidget();
	}

	void TrendWidget::updatePixmap(const QImage& image, double /*scaleFactor*/)
	{
		static int updatePixmapCounter = 0;
		qDebug() << "updatePixmapCounter " << updatePixmapCounter++;

		m_pixmap = QPixmap::fromImage(image);

		update();
		return;
	}

}
