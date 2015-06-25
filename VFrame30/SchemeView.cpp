#include "Stable.h"
#include "SchemeView.h"

namespace VFrame30
{
	SchemeView::SchemeView(QWidget *parent) :
		QWidget(parent)
	{
		init();
	}

	SchemeView::SchemeView(std::shared_ptr<Scheme>& scheme, QWidget* parent /*= 0*/) :
		QWidget(parent),
		m_scheme(scheme)
	{
		init();
	}

	void SchemeView::init()
	{
		setMouseTracking(true);
	}

	std::shared_ptr<Scheme>& SchemeView::scheme()
	{
		return m_scheme;
	}

	std::shared_ptr<Scheme> SchemeView::scheme() const
	{
		return m_scheme;
	}

	void SchemeView::setScheme(std::shared_ptr<Scheme>& scheme, bool repaint)
	{
		assert(scheme.get() != nullptr);
		m_scheme = scheme;

		setZoom(zoom(), repaint);		// Adhust sliders, widget etc.
	}

	void SchemeView::mouseMoveEvent(QMouseEvent* event)
	{
		// If any contrtol key is pressed, pass control further
		//
		if (event->buttons().testFlag(Qt::LeftButton) == true ||
			event->buttons().testFlag(Qt::RightButton) == true ||
			event->buttons().testFlag(Qt::MidButton) == true)
		{
			unsetCursor();		// set cursor to parent cursor

			event->ignore();
			return;
		}

		if (scheme() == nullptr)
		{
			// Scheme is not loaded
			//
			return;
		}

		// If the mouse cursor is over SchmeItem with acceptClick then set HandCursor
		//
		QPointF docPoint;

		bool convertResult = MousePosToDocPoint(event->pos(), &docPoint);
		if (convertResult == false)
		{
			unsetCursor();
			return;
		}
				
		double x = docPoint.x();
		double y = docPoint.y();
		
		for (auto layer = scheme()->Layers.crbegin(); layer != scheme()->Layers.crend(); layer++)
		{
			const SchemeLayer* pLayer = layer->get();

			if (pLayer->show() == false)
			{
				continue;
			}

			for (auto vi = pLayer->Items.crbegin(); vi != pLayer->Items.crend(); vi++)
			{
				const std::shared_ptr<SchemeItem>& item = *vi;

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

	void SchemeView::paintEvent(QPaintEvent*)
	{
		if (scheme().get() == nullptr)
		{
			return;
		}

		// --
		//
		QPainter p(this);
		CDrawParam drawParam(&p, scheme()->gridSize(), scheme()->pinGridStep());

		// Calc size
		//
		int widthInPixel = scheme()->GetDocumentWidth(p.device()->logicalDpiX(), zoom());
		int heightInPixel = scheme()->GetDocumentHeight(p.device()->logicalDpiY(), zoom());

		// Clear device
		//
		p.fillRect(QRectF(0, 0, widthInPixel + 1, heightInPixel + 1), QColor(0xB0, 0xB0, 0xB0));
		p.setRenderHint(QPainter::Antialiasing);

		// Ajust QPainter
		//
		Ajust(&p, 0, 0);

		// Draw Scheme
		//
		QRectF clipRect(0, 0, scheme()->docWidth(), scheme()->docHeight());

		scheme()->Draw(&drawParam, clipRect);

		// Ending
		//
		return;
	}

	void SchemeView::Ajust(QPainter* painter, double startX, double startY) const
	{
		// Set transform matrix
		//
		painter->resetTransform();

		if (m_scheme->unit() == SchemeUnit::Inch)
		{
			painter->translate(startX + 0.5, startY + 0.5);
			painter->scale(
				(double)painter->device()->logicalDpiX() * zoom() / 100.0,
				(double)painter->device()->logicalDpiY() * zoom() / 100.0);
		}
		else
		{
			startX = CUtils::Round(startX) + 0.5;
			startY = CUtils::Round(startY) + 0.5;

			painter->translate(startX, startY);
			painter->scale(zoom() / 100.0, zoom() / 100.0);
		}

		return;
	}

	bool SchemeView::MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, int dpiX /*= 0*/, int dpiY /*= 0*/)
	{
		if (pDestDocPos == nullptr)
		{
			assert(pDestDocPos != nullptr);
			return false;
		}

		int x = mousePos.x();
		int y = mousePos.y();

		if (scheme()->unit() == SchemeUnit::Display)
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

	double SchemeView::zoom() const
	{
		return m_zoom;
	}

	void SchemeView::setZoom(double value, bool repaint /*= true*/, int dpiX /*= 0*/, int dpiY /*= 0*/)
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
		int widthInPixel = scheme()->GetDocumentWidth(dpiX, m_zoom);
		int heightInPixel = scheme()->GetDocumentHeight(dpiY, m_zoom);

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
