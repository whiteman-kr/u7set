#include "Stable.h"
#include "SchemaView.h"
#include <QPdfWriter>

namespace VFrame30
{
	SchemaView::SchemaView(QWidget *parent) :
		QWidget(parent)
	{
		init();
	}

	SchemaView::SchemaView(std::shared_ptr<Schema>& schema, QWidget* parent /*= 0*/) :
		QWidget(parent),
		m_schema(schema)
	{
		init();
	}

	void SchemaView::init()
	{
		setMouseTracking(true);
	}

	std::shared_ptr<Schema>& SchemaView::schema()
	{
		return m_schema;
	}

	std::shared_ptr<Schema> SchemaView::schema() const
	{
		return m_schema;
	}

	void SchemaView::setSchema(std::shared_ptr<Schema>& schema, bool repaint)
	{
		assert(schema.get() != nullptr);
		m_schema = schema;

		setZoom(zoom(), repaint);		// Adhust sliders, widget etc.

		emit signal_schemaChanged(schema.get());
	}

	void SchemaView::mouseMoveEvent(QMouseEvent* event)
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

		if (schema() == nullptr)
		{
			// Schema is not loaded
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
		
		for (auto layer = schema()->Layers.crbegin(); layer != schema()->Layers.crend(); layer++)
		{
			const SchemaLayer* pLayer = layer->get();

			if (pLayer->show() == false)
			{
				continue;
			}

			for (auto vi = pLayer->Items.crbegin(); vi != pLayer->Items.crend(); vi++)
			{
				const std::shared_ptr<SchemaItem>& item = *vi;

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

	void SchemaView::paintEvent(QPaintEvent*)
	{
		if (schema().get() == nullptr)
		{
			return;
		}

		QPainter p(this);
		CDrawParam drawParam(&p, schema().get(), schema()->gridSize(), schema()->pinGridStep());

		draw(drawParam);

		return;
	}

	void SchemaView::draw(CDrawParam& drawParam)
	{
		if (schema().get() == nullptr)
		{
			return;
		}

		QPainter* p = drawParam.painter();

		// Calc size
		//
		int widthInPixel = schema()->GetDocumentWidth(p->device()->physicalDpiX(), zoom());
		int heightInPixel = schema()->GetDocumentHeight(p->device()->physicalDpiY(), zoom());

		// Clear device
		//
		p->fillRect(QRectF(0, 0, widthInPixel + 1, heightInPixel + 1), QColor(0xB0, 0xB0, 0xB0));

		if (p->device()->physicalDpiX() <= 96)
		{
			// If higher then 96 then most likely it is 4K display, no need to use Antialiasing
			// Note, that font will be antialiased in anyway
			//
			p->setRenderHint(QPainter::Antialiasing);
		}

		// Ajust QPainter
		//
		Ajust(p, 0, 0, zoom());

		// Draw Schema
		//
		QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());

		schema()->Draw(&drawParam, clipRect);
	}

	void SchemaView::Ajust(QPainter* painter, double startX, double startY, double zoom) const
	{
		// Set transform matrix
		//
		painter->resetTransform();

		if (m_schema->unit() == SchemaUnit::Inch)
		{
			painter->translate(startX + 0.5, startY + 0.5);
			painter->scale(
				(double)painter->device()->physicalDpiX() * zoom / 100.0,
				(double)painter->device()->physicalDpiY() * zoom / 100.0);
		}
		else
		{
			startX = CUtils::Round(startX) + 0.5;
			startY = CUtils::Round(startY) + 0.5;

			painter->translate(startX, startY);
			painter->scale(zoom / 100.0, zoom / 100.0);
		}

		return;
	}

	void SchemaView::exportToPdf(QString fileName) const
	{
		if (schema().get() == nullptr)
		{
			return;
		}

		// --
		//
		QPdfWriter pdfWriter(fileName);

		pdfWriter.setTitle(schema()->caption());

		QPageSize pageSize;
		double pageWidth = schema()->docWidth();
		double pageHeight = schema()->docHeight();

		if (m_schema->unit() == SchemaUnit::Inch)
		{
			pageSize = QPageSize(QSizeF(pageWidth, pageHeight), QPageSize::Inch);
		}
		else
		{
			assert(m_schema->unit() == SchemaUnit::Display);
			pageSize = QPageSize(QSize(static_cast<int>(pageWidth), static_cast<int>(pageHeight)));

			pdfWriter.setResolution(72);	// 72 is from enum QPageLayout::Unit help,
											// QPageLayout::Point	1	1/!!! 72th !!!! of an inch
		}

		pdfWriter.setPageSize(pageSize);
		pdfWriter.setPageMargins(QMarginsF(0, 0, pageWidth, pageHeight));

		// --
		//
		QPainter p(&pdfWriter);
		CDrawParam drawParam(&p, schema().get(), schema()->gridSize(), schema()->pinGridStep());

		// Calc size
		//
		int widthInPixel = schema()->GetDocumentWidth(p.device()->physicalDpiX(), 100.0);		// Export 100% zoom
		int heightInPixel = schema()->GetDocumentHeight(p.device()->physicalDpiY(), 100.0);		// Export 100% zoom

		// Clear device
		//
		p.fillRect(QRectF(0, 0, widthInPixel + 1, heightInPixel + 1), QColor(0xB0, 0xB0, 0xB0));
		p.setRenderHint(QPainter::Antialiasing);

		// Ajust QPainter
		//
		//Ajust(&p, 0, 0, 100.0);		// Export 100% zoom
		Ajust(&p, 0, 0, 100.0);		// Export 100% zoom

		// Draw Schema
		//
		QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());

		schema()->Draw(&drawParam, clipRect);

		// Ending
		//

		return;
	}

	bool SchemaView::MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, int dpiX /*= 0*/, int dpiY /*= 0*/)
	{
		if (pDestDocPos == nullptr)
		{
			assert(pDestDocPos != nullptr);
			return false;
		}

		int x = mousePos.x();
		int y = mousePos.y();

		if (schema()->unit() == SchemaUnit::Display)
		{
			pDestDocPos->setX(x / (m_zoom / 100.0));
			pDestDocPos->setY(y / (m_zoom / 100.0));
		}
		else
		{
			dpiX = dpiX == 0 ? physicalDpiX() : dpiX;
			dpiY = dpiY == 0 ? physicalDpiY() : dpiY;

			pDestDocPos->setX(x / (dpiX * (m_zoom / 100.0)));
			pDestDocPos->setY(y / (dpiY * (m_zoom / 100.0)));
		}

		return true;
	}

	// Properties
	//

	double SchemaView::zoom() const
	{
		return m_zoom;
	}

	void SchemaView::setZoom(double value, bool repaint /*= true*/, int dpiX /*= 0*/, int dpiY /*= 0*/)
	{
		value = value > 500 ? 500 : value;
		value = value < 50 ? 50 : value;

		m_zoom = value;

		// Calc DPI
		//
		dpiX = (dpiX == 0) ? physicalDpiX() : dpiX;
		dpiY = (dpiY == 0) ? physicalDpiY() : dpiY;

		// resize widget
		//
		int widthInPixel = schema()->GetDocumentWidth(dpiX, m_zoom);
		int heightInPixel = schema()->GetDocumentHeight(dpiY, m_zoom);

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
