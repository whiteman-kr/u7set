#include "Stable.h"
#include "VideoFrameView.h"

namespace VFrame30
{
	VideoFrameView::VideoFrameView(QWidget *parent) :
		QWidget(parent)
	{
		init();
	}

	VideoFrameView::VideoFrameView(std::shared_ptr<CVideoFrame> &videoFrame, QWidget* parent /*= 0*/) :
		QWidget(parent)
	{
		this->m_videoFrame = videoFrame;
		init();
	}

	void VideoFrameView::init()
	{
		setMouseTracking(true);

		m_zoom = 100;
	}

	std::shared_ptr<CVideoFrame>& VideoFrameView::videoFrame()
	{
		return m_videoFrame;
	}

	std::shared_ptr<CVideoFrame> VideoFrameView::videoFrame() const
	{
		return m_videoFrame;
	}

	void VideoFrameView::setVideoFrame(std::shared_ptr<CVideoFrame>& videoFrame, bool repaint)
	{
		assert(videoFrame.get() != nullptr);
		m_videoFrame = videoFrame;

		setZoom(zoom(), repaint);		// Adhust sliders, widget etc.
	}

	void VideoFrameView::mouseMoveEvent(QMouseEvent* event)
	{
		// Если прижата какая либо кнопка, отдать управление дальше (VideoFrameWidget?)
		//
		if (event->buttons().testFlag(Qt::LeftButton) == true ||
			event->buttons().testFlag(Qt::RightButton) == true ||
			event->buttons().testFlag(Qt::MidButton) == true)
		{
			unsetCursor();		// set cursor to parent cursor

			event->ignore();
			return;
		}

		// Eсли курсор над элементом у которого установлен acceptClick, то заменить курор на руку
		//
		std::shared_ptr<CVideoFrame>& vf = videoFrame();
		if (vf.get() == nullptr)
		{
			assert(vf.get() != nullptr);
			return;
		}

		QPointF docPoint;

		bool convertResult = MousePosToDocPoint(event->pos(), &docPoint);
		if (convertResult == false)
		{
			unsetCursor();
			return;
		}
				
		double x = docPoint.x();
		double y = docPoint.y();
		
		for (auto layer = vf->Layers.crbegin(); layer != vf->Layers.crend(); layer++)
		{
			const CVideoLayer* pLayer = layer->get();

			if (pLayer->show() == false)
			{
				continue;
			}

			for (auto vi = pLayer->Items.crbegin(); vi != pLayer->Items.crend(); vi++)
			{
				const std::shared_ptr<CVideoItem>& item = *vi;

				if (item->acceptClick() == true && item->IsIntersectPoint(x, y) == true && item->clickScript().isEmpty() == false)
				{
					setCursor(Qt::PointingHandCursor);
					event->accept();
					return;
				}
			}
		}

		unsetCursor();

		event->ignore();
		return;
	}

	void VideoFrameView::paintEvent(QPaintEvent*)
	{
		if (videoFrame().get() == nullptr)
		{
			return;
		}

		// --
		//
		QPainter p(this);
		CDrawParam drawParam(&p);

		// Calc size
		//
		int widthInPixel = videoFrame()->GetDocumentWidth(p.device()->logicalDpiX(), zoom());
		int heightInPixel = videoFrame()->GetDocumentHeight(p.device()->logicalDpiY(), zoom());

		// Clear device
		//
		p.fillRect(QRectF(0, 0, widthInPixel + 1, heightInPixel + 1), QColor(0xB0, 0xB0, 0xB0));
		p.setRenderHint(QPainter::Antialiasing);

		// Ajust QPainter
		//
		Ajust(&p, 0, 0);

		// Draw VideoFrame
		//
		QRectF clipRect(0, 0, videoFrame()->docWidth(), videoFrame()->docHeight());

		videoFrame()->Draw(&drawParam, clipRect);

		// Ending
		//
		return;
	}

	void VideoFrameView::Ajust(QPainter* painter, double startX, double startY) const
	{
		// Set transform matrix
		//
		painter->resetTransform();

		if (m_videoFrame->unit() == SchemeUnit::Inch)
		{
			painter->translate(startX + 0.5, startY + 0.5);
			painter->scale(
				(double)painter->device()->logicalDpiX() * zoom() / 100.0,
				(double)painter->device()->logicalDpiY() * zoom() / 100.0);
		}
		else
		{
			startX = VFrame30::CVFrameUtils::Round(startX) + 0.5;
			startY = VFrame30::CVFrameUtils::Round(startY) + 0.5;

			painter->translate(startX, startY);
			painter->scale(zoom() / 100.0, zoom() / 100.0);
		}

		return;
	}

	bool VideoFrameView::MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, int dpiX /*= 0*/, int dpiY /*= 0*/)
	{
		if (pDestDocPos == nullptr)
		{
			assert(pDestDocPos != nullptr);
			return false;
		}

		int x = mousePos.x();
		int y = mousePos.y();

		if (videoFrame()->unit() == SchemeUnit::Display)
		{
			pDestDocPos->setX(x / (m_zoom / 100.0));
			pDestDocPos->setY(y / (m_zoom / 100.0));
		}
		else
		{
			dpiX = dpiX == 0 ? logicalDpiX() : dpiX;
			dpiY = dpiY == 0 ? logicalDpiY() : dpiY;

			pDestDocPos->setX(x / (dpiX * (m_zoom / 100.0)));
			pDestDocPos->setY(y / (dpiY * (m_zoom / 100.0)));
		}

		return true;
	}

	// Properties
	//

	double VideoFrameView::zoom() const
	{
		return m_zoom;
	}

	void VideoFrameView::setZoom(double value, bool repaint /*= true*/, int dpiX /*= 0*/, int dpiY /*= 0*/)
	{
		value = value > 500 ? 500 : value;
		value = value < 50 ? 50 : value;

		m_zoom = value;

		// Calc DPI
		//
		dpiX = (dpiX == 0) ? logicalDpiX() : dpiX;
		dpiY = (dpiY == 0) ? logicalDpiY() : dpiY;

		// resize widget
		//
		int widthInPixel = videoFrame()->GetDocumentWidth(dpiX, m_zoom);
		int heightInPixel = videoFrame()->GetDocumentHeight(dpiY, m_zoom);

		QSize scaledPixelSize;

		scaledPixelSize.setWidth(widthInPixel);
		scaledPixelSize.setHeight(heightInPixel);

		setMinimumSize(scaledPixelSize);

		resize(scaledPixelSize);

		if (repaint == true)
		{
			this->repaint();
		}

		return;
	}

}
