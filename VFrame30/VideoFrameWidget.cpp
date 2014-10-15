#include "Stable.h"
#include "VideoFrameManager.h"
#include "VideoFrameWidget.h"
#include "Scheme.h"
#include "SchemeView.h"

namespace VFrame30
{

	// VideoFrameWidget
	//
	VideoFrameBaseWidget::VideoFrameBaseWidget(std::shared_ptr<Scheme> videoFrame)
	{
		horzScrollBarValue = 0;
		vertScrollBarValue = 0;

		setBackgroundRole(QPalette::Dark);
		setAlignment(Qt::AlignCenter);
		setMouseTracking(true);

		// --
		//
		m_pVideoFrameView = new SchemeView(videoFrame, this);

		m_pVideoFrameView->setZoom(100);
		setWidget(m_pVideoFrameView);

		// --
		//
		return;
	}

	VideoFrameBaseWidget::~VideoFrameBaseWidget()
	{
	}

	void VideoFrameBaseWidget::mousePressEvent(QMouseEvent* event)
	{
		if (event->buttons().testFlag(Qt::MidButton))
		{
			// Enter to scrolling mode
			//
			mousePos = event->pos();

			horzScrollBarValue = horizontalScrollBar()->value();
			vertScrollBarValue = verticalScrollBar()->value();

			setCursor(Qt::OpenHandCursor);

			event->accept();
			return;
		}

		event->ignore();
		return;
	}

	void VideoFrameBaseWidget::mouseReleaseEvent(QMouseEvent* event)
	{
		unsetCursor();
		event->ignore();
	}

	void VideoFrameBaseWidget::mouseMoveEvent(QMouseEvent* event)
	{
		if (event->buttons().testFlag(Qt::MidButton) == true)
		{
			int dx = event->x() - mousePos.x();
			int dy = event->y() - mousePos.y();

			horizontalScrollBar()->setValue(horzScrollBarValue - dx);
			verticalScrollBar()->setValue(vertScrollBarValue - dy);

			event->accept();
			return;
		}

		event->ignore();
		return;
	}

	void VideoFrameBaseWidget::wheelEvent(QWheelEvent* event)
	{
		if (widget() == nullptr)
		{
			return;
		}

		// While midButton is pressed, this is move mode, don't change zoom
		//
		if (event->buttons().testFlag(Qt::MidButton))
		{
			return;
		}

		int numDegrees = event->delta() / 8;
		int numSteps = numDegrees / 15;

		if (numSteps != 0)
		{
			double zoom = videoFrameView()->zoom() + numSteps * 10;

			QPointF oldDocPos;
			MousePosToDocPoint(event->pos(), &oldDocPos);

			videoFrameView()->setZoom(zoom, false);

			QPointF newDocPos;
			MousePosToDocPoint(event->pos(), &newDocPos);

			//
			//
			QPointF dPos = (newDocPos - oldDocPos);

			//   (),          ( )
			//
			int newHorzValue = 0;
			int newVertValue = 0;

			switch (videoFrame()->unit())
			{
			case SchemeUnit::Display:
				newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * zoom / 100.0);
				newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * zoom / 100.0);
				break;
			case SchemeUnit::Inch:
				newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * (zoom / 100.0) * logicalDpiX());
				newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * (zoom / 100.0) * logicalDpiY());
				break;
			default:
				assert(false);
			}

			horizontalScrollBar()->setValue(newHorzValue);
			verticalScrollBar()->setValue(newVertValue);
		}

		event->accept();
		return;
	}

	std::shared_ptr<Scheme>& VideoFrameBaseWidget::videoFrame()
	{
		SchemeView* ptr = dynamic_cast<SchemeView*>(widget());
		if (ptr == nullptr)
		{
			assert(ptr != nullptr);
			static std::shared_ptr<Scheme> nullSharedPtr;
			return nullSharedPtr;
		}

		return ptr->videoFrame();
	}

	SchemeView* VideoFrameBaseWidget::videoFrameView()
	{
		assert(dynamic_cast<SchemeView*>(widget()) != nullptr);

		if (m_pVideoFrameView == nullptr)
		{
			assert(m_pVideoFrameView != nullptr);
			return nullptr;
		}

		return m_pVideoFrameView;
	}

	bool VideoFrameBaseWidget::MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, int dpiX /*= 0*/, int dpiY /*= 0*/)
	{
		if (pDestDocPos == nullptr)
		{
			assert(pDestDocPos != nullptr);
			return false;
		}

		dpiX = dpiX == 0 ? logicalDpiX() : dpiX;
		dpiY = dpiY == 0 ? logicalDpiY() : dpiY;

		double zoom = videoFrameView()->zoom();

		int widthInPixels = videoFrame()->GetDocumentWidth(dpiX, zoom);
		int heightInPixels = videoFrame()->GetDocumentHeight(dpiY, zoom);

		int startX = 0;
		int startY = 0;

		if (rect().width() >= widthInPixels)
		{
			startX = (rect().width() - widthInPixels) / 2;
		}
		else
		{
			startX = -horizontalScrollBar()->value();
		}

		if (rect().height() >= heightInPixels)
		{
			startY = (rect().height() - heightInPixels) / 2;
		}
		else
		{
			startY = -verticalScrollBar()->value();
		}

		int x = mousePos.x() - startX;
		int y = mousePos.y() - startY;

		if (videoFrame()->unit() == SchemeUnit::Display)
		{
			pDestDocPos->setX(x / (zoom / 100.0));
			pDestDocPos->setY(y / (zoom / 100.0));
		}
		else
		{
			pDestDocPos->setX(x / (dpiX * (zoom / 100.0)));
			pDestDocPos->setY(y / (dpiY * (zoom / 100.0)));
		}

		return true;
	}

	double VideoFrameBaseWidget::zoom() const
	{
		if (m_pVideoFrameView == nullptr)
		{
			assert(m_pVideoFrameView != nullptr);
			return 0;
		}
		
		return m_pVideoFrameView->zoom();
	}

	void VideoFrameBaseWidget::setZoom(double zoom, int horzScrollValue /*= -1*/, int vertScrollValue /*= -1*/)
	{
		QPoint widgetCenterPoint(size().width() / 2, size().height() / 2);

		QPointF oldDocPos;
		MousePosToDocPoint(widgetCenterPoint, &oldDocPos);

		videoFrameView()->setZoom(zoom, false);

		QPointF newDocPos;
		MousePosToDocPoint(widgetCenterPoint, &newDocPos);

		// --
		//
		QPointF dPos = (newDocPos - oldDocPos);

		// --
		//
		int newHorzValue = 0;
		int newVertValue = 0;

		switch (videoFrame()->unit())
		{
		case SchemeUnit::Display:
			newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * zoom / 100.0);
			newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * zoom / 100.0);
			break;
		case SchemeUnit::Inch:
			newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * (zoom / 100.0) * logicalDpiX());
			newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * (zoom / 100.0) * logicalDpiY());
			break;
		default:
			assert(false);
		}

		horizontalScrollBar()->setValue(horzScrollValue == -1 ? newHorzValue : horzScrollValue);
		verticalScrollBar()->setValue(vertScrollValue == -1 ? newVertValue : vertScrollValue);

		return;
	}

}
