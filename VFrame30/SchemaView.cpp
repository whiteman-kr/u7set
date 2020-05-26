#include "SchemaView.h"
#include "Schema.h"
#include "SchemaItemControl.h"
#include "DrawParam.h"

namespace VFrame30
{
	SchemaView::SchemaView(QWidget* parent) :
		SchemaView(std::shared_ptr<Schema>(), parent)
	{
	}

	SchemaView::SchemaView(std::shared_ptr<Schema> schema, QWidget* parent /*= 0*/) :
		QWidget(parent),
		m_schema(schema)
	{
		setMouseTracking(true);
		return;
	}

	void SchemaView::updateControlWidgets(bool editMode)
	{
		// Find all SchemaItemControl
		//
		std::map<QUuid, std::shared_ptr<VFrame30::SchemaItemControl>> controlItems;

		for (std::shared_ptr<VFrame30::SchemaLayer> layer : schema()->Layers)
		{
			// Control items on Compile layer are ok, but on other layers they must be disabled (grayed)
			//
			for (SchemaItemPtr& item : layer->Items)
			{
				if (item->isControl() == false)
				{
					continue;
				}

				VFrame30::SchemaItemControl* controlItem = item->toType<VFrame30::SchemaItemControl>();
				if (controlItem == nullptr)
				{
					Q_ASSERT(controlItem);
					continue;
				}

				controlItems[item->guid()] = std::dynamic_pointer_cast<SchemaItemControl>(item);
			}
		}

		// Update all children
		//
		QObjectList childWidgets = children();							// Don't make childWidgets as a reference, as we change this list in the loop

		for (QObject* childObject : childWidgets)
		{
			QWidget* childWidget = dynamic_cast<QWidget*>(childObject);

			if (childWidget == nullptr)
			{
				assert(dynamic_cast<QWidget*>(childObject) != nullptr);
				continue;
			}

			QString objectName = childWidget->objectName();
			QUuid widgetUuid = QUuid(objectName);

			if (widgetUuid.isNull() == true)
			{
				continue;
			}

			auto foundIt = controlItems.find(widgetUuid);

			if (foundIt == controlItems.end())
			{
				// Apparently SchemaItemControl was deleted
				//
				delete childWidget;
				continue;
			}

			std::shared_ptr<VFrame30::SchemaItemControl> controlItem = foundIt->second;

			controlItem->updateWdgetPosAndSize(childWidget, zoom());

			if (editMode == true)
			{
				controlItem->updateWidgetProperties(childWidget);
			}

			controlItems.erase(widgetUuid);
		}

		// Create new items
		//
		for (auto controlItemPair : controlItems)
		{
			std::shared_ptr<VFrame30::SchemaItemControl> controlItem = controlItemPair.second;

			QWidget* childWidget = controlItem->createWidget(this, editMode);
			assert(childWidget);

			Q_UNUSED(childWidget);
		}
	}

	void SchemaView::deleteControlWidgets()
	{
		// Find all control items
		//
		std::map<QUuid, std::shared_ptr<VFrame30::SchemaItemControl>> controlItems;

		for (std::shared_ptr<VFrame30::SchemaLayer> layer : schema()->Layers)
		{
			for (SchemaItemPtr& item : layer->Items)
			{
				if (item->isControl() == false)
				{
					continue;
				}

				VFrame30::SchemaItemControl* controlItem = item->toType<VFrame30::SchemaItemControl>();
				if (controlItem == nullptr)
				{
					Q_ASSERT(controlItem);
					continue;
				}

				controlItems[item->guid()] = std::dynamic_pointer_cast<SchemaItemControl>(item);
			}
		}

		// Remove all control widgets
		//
		QObjectList childWidgets = children();							// Don't make childWidgets as a reference, as we change this list in the loop

		for (QObject* childObject : childWidgets)
		{
			QWidget* childWidget = dynamic_cast<QWidget*>(childObject);

			if (childWidget == nullptr)
			{
				assert(dynamic_cast<QWidget*>(childObject) != nullptr);
				continue;
			}

			QString objectName{childWidget->objectName()};
			QUuid widgetUuid{objectName};

			if (widgetUuid.isNull() == true)
			{
				continue;
			}

			if (auto foundIt = controlItems.find(widgetUuid);
				foundIt != controlItems.end())
			{
				delete childWidget;
			}
		}

		return;
	}

	std::shared_ptr<Schema> SchemaView::schema()
	{
		return m_schema;
	}

	std::shared_ptr<Schema> SchemaView::schema() const
	{
		return m_schema;
	}

	void SchemaView::setSchema(std::shared_ptr<Schema> schema, bool repaint)
	{
		assert(schema.get() != nullptr);
		m_schema = schema;

		setZoom(zoom(), repaint);		// Adjust sliders, widget etc.

		emit signal_schemaChanged(schema.get());
		return;
	}

	void SchemaView::setSchemaInternal(std::shared_ptr<Schema> schema)
	{
		// Use this when yoo dont need to update zoom, sliders, etc
		//
		assert(schema.get() != nullptr);
		m_schema = schema;
		return;
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

				if (item->acceptClick() == true && item->isIntersectPoint(x, y) == true && item->clickScript().isEmpty() == false)
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
		CDrawParam drawParam(&p, schema().get(), this, schema()->gridSize(), schema()->pinGridStep());

		draw(drawParam);

		return;
	}

	void SchemaView::draw(CDrawParam& drawParam)
	{
		if (schema() == nullptr)
		{
			return;
		}

		updateControlWidgets(drawParam.isEditMode());

		// --
		//
		QPainter* p = drawParam.painter();

		// Calc size
		//
		int widthInPixel = schema()->GetDocumentWidth(p->device()->logicalDpiX(), zoom());
		int heightInPixel = schema()->GetDocumentHeight(p->device()->logicalDpiY(), zoom());

		// Clear device
		//
		p->fillRect(QRectF(0, 0, widthInPixel + 1, heightInPixel + 1), QColor(0xB0, 0xB0, 0xB0));

		if (p->device()->logicalDpiX() <= 96)
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
				(double)painter->device()->logicalDpiX() * zoom / 100.0,
				(double)painter->device()->logicalDpiY() * zoom / 100.0);
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

	void SchemaView::exportToPdf(QString fileName, const Session& session, bool infoMode) const
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
		CDrawParam drawParam(&p, schema().get(), this, schema()->gridSize(), schema()->pinGridStep());

		drawParam.setInfoMode(infoMode);
		drawParam.session() = session;

		// Calc size
		//
		int widthInPixel = schema()->GetDocumentWidth(p.device()->logicalDpiX(), 100.0);		// Export 100% zoom
		int heightInPixel = schema()->GetDocumentHeight(p.device()->logicalDpiY(), 100.0);		// Export 100% zoom

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
			dpiX = dpiX == 0 ? logicalDpiX() : dpiX;
			dpiY = dpiY == 0 ? logicalDpiY() : dpiY;

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

	double SchemaView::setZoom(double value, bool repaint /*= true*/, int dpiX /*= 0*/, int dpiY /*= 0*/)
	{
		// Calc DPI
		//
		dpiX = (dpiX == 0) ? logicalDpiX() : dpiX;
		dpiY = (dpiY == 0) ? logicalDpiY() : dpiY;

		// if value is 0 then fit page into parent
		//
		if (value == 0)
		{
			QWidget* viewportWidget = this->parentWidget();		// Viewport can be real from QAbstractScrollArea or just any widget
			QAbstractScrollArea* abstractScrollArea = qobject_cast<QAbstractScrollArea*>(viewportWidget->parentWidget());

			if (viewportWidget == nullptr)
			{
				Q_ASSERT(viewportWidget);
				value = 100;
			}
			else
			{
				QSize viewportSize;

				if (abstractScrollArea != nullptr)
				{
					viewportSize = abstractScrollArea->maximumViewportSize();
				}
				else
				{
					viewportSize = viewportWidget->size();
				}

				// Scale to fit viewportWidget
				//
				double vertScaleFactor = 1.0;
				double horzScaleFactor = 1.0;

				if (schema()->unit() == SchemaUnit::Display)
				{
					horzScaleFactor = (viewportSize.width() * 0.99) / schema()->docWidth();
					vertScaleFactor = (viewportSize.height() * 0.99) / schema()->docHeight();
				}
				else
				{
					horzScaleFactor = (viewportSize.width() * 0.99) / (schema()->docWidth() * dpiX);
					vertScaleFactor = (viewportSize.height() * 0.99) / (schema()->docHeight() * dpiY);
				}

				value = std::min(vertScaleFactor, horzScaleFactor) * 100.0;
			}
		}
		else
		{
			value = qBound(30.0, value, 500.0);
		}

		m_zoom = value;

		// resize widget
		//
		int widthInPixel = schema()->GetDocumentWidth(dpiX, m_zoom);
		int heightInPixel = schema()->GetDocumentHeight(dpiY, m_zoom);

		QSize scaledPixelSize;

		scaledPixelSize.setWidth(widthInPixel);
		scaledPixelSize.setHeight(heightInPixel);

		if (minimumSize() != scaledPixelSize)
		{
			setMinimumSize(scaledPixelSize);
		}

		if (size() != scaledPixelSize)
		{
			resize(scaledPixelSize);
		}

		if (repaint == true)
		{
			this->repaint();
		}

		return m_zoom;
	}

	const Session& SchemaView::session() const
	{
		return m_session;
	}

	Session& SchemaView::session()
	{
		return m_session;
	}

}
