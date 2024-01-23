#include "SchemaView.h"
#include "Schema.h"
#include "SchemaItemControl.h"
#include "SchemaLayer.h"

namespace VFrame30
{

	SchemaView::SchemaView()
	{
	}

	SchemaView::SchemaView(std::shared_ptr<Schema> schema) :
		m_schema(schema)
	{
	}

	void SchemaView::Ajust(QPainter* painter, SchemaUnit units, double startX, double startY, double zoom)
	{
		int dpix = painter->device()->physicalDpiX();
		int dpiy = painter->device()->physicalDpiY();
		double dpr = painter->device()->devicePixelRatioF();

		return SchemaView::Ajust(painter, dpix, dpiy, dpr, units, startX, startY, zoom);
	}

	void SchemaView::Ajust(QPainter* painter, int dpiX, int dpiY, double devicePixelRatioF, SchemaUnit units, double startX, double startY, double zoom)
	{
		double dpix = static_cast<double>(dpiX);
		double dpiy = static_cast<double>(dpiY);

		// Set transform matrix
		//
		painter->resetTransform();

		zoom /= 100.0;

		if (units == SchemaUnit::Inch)
		{
			startX = startX + 0.5 / devicePixelRatioF;
			startY = startY + 0.5 / devicePixelRatioF;

			double scalex = dpix * zoom;
			double scaley = dpiy * zoom;

			painter->translate(startX, startY);
			painter->scale(scalex, scaley);
		}
		else
		{
			startX = VFrame30::Round(startX) + 0.5 / devicePixelRatioF;
			startY = VFrame30::Round(startY) + 0.5 / devicePixelRatioF;

			double scalex = 1.0 / devicePixelRatioF * zoom;
			double scaley = 1.0 / devicePixelRatioF * zoom;

			painter->translate(startX, startY);
			painter->scale(scalex, scaley);
		}
	}

	double SchemaView::realDpiX(const QPaintDevice* device) const
	{
		// Drawing can be performed in other device, not in windows, for example to pdf, that's why device is required
		//
		Q_ASSERT(device);
		return device->physicalDpiX() * device->devicePixelRatioF();
	}

	double SchemaView::realDpiY(const QPaintDevice* device) const
	{
		// Drawing can be performed in other device, not in windows, for example to pdf, that's why device is required
		//
		Q_ASSERT(device);
		return device->physicalDpiY() * device->devicePixelRatioF();
	}

	void SchemaView::setSchema(std::shared_ptr<Schema> schema, bool repaint)
	{
		Q_UNUSED(repaint);
		Q_ASSERT(false); // This function is a stub and should be overriden in derived class

		setSchemaInternal(schema);
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

	VFrame30::Schema* SchemaView::schema()
	{
		return m_schema.get();
	}

	const VFrame30::Schema* SchemaView::schema() const
	{
		return m_schema.get();
	}

	std::shared_ptr<VFrame30::Schema> SchemaView::schemaSharedPtr()
	{
		return m_schema;
	}

	std::shared_ptr<VFrame30::Schema> SchemaView::schemaSharedPtr() const
	{
		return m_schema;
	}

	// Properties
	//
	double SchemaView::zoom() const
	{
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


	//
	// SchemaViewWidget
	//
	SchemaViewWidget::SchemaViewWidget(QWidget* parent) :
		SchemaViewWidget(std::shared_ptr<Schema>(), parent)
	{
	}

	SchemaViewWidget::SchemaViewWidget(std::shared_ptr<Schema> schema, QWidget* parent /*= 0*/) :
		QWidget(parent),
		SchemaView(schema)
	{
		setAttribute(Qt::WA_OpaquePaintEvent);
		setMouseTracking(true);
		return;
	}

	void SchemaViewWidget::updateControlWidgets(bool editMode)
	{
		// Find all SchemaItemControl
		//
		std::map<QUuid, std::shared_ptr<VFrame30::SchemaItemControl>> controlItems;

		for (const auto& layer : schema()->layers())
		{
			// Control items on Compile layer are ok, but on other layers they must be disabled (grayed)
			//
			for (const auto& item : layer->items())
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
		// !!! Do not make childWidgets as a reference, as we change this list in the loop !!!
		//
		QObjectList childWidgets = children();

		for (QObject* childObject : childWidgets)
		{
			QWidget* childWidget = dynamic_cast<QWidget*>(childObject);

			if (childWidget == nullptr)
			{
				assert(dynamic_cast<QWidget*>(childObject) != nullptr || dynamic_cast<TuningController*>(childObject) != nullptr);
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

				// childWidget cannot just be deleted, as some events for this widget still can be in the message queue.
				// deleteLater() will delete object.
				//
				childWidget->setObjectName("");
				childWidget->hide();
				childWidget->deleteLater();

				continue;
			}

			std::shared_ptr<VFrame30::SchemaItemControl> controlItem = foundIt->second;

			controlItem->updateWidgetPosAndSize(childWidget, zoom());

			if (editMode == true)
			{
				controlItem->updateWidgetProperties(childWidget, editMode);
			}

			controlItems.erase(widgetUuid);
		}

		// Create new items
		//
		for (const auto& controlItemPair : controlItems)
		{
			std::shared_ptr<VFrame30::SchemaItemControl> controlItem = controlItemPair.second;

			QWidget* childWidget = controlItem->createWidget(this, editMode, zoom());
			assert(childWidget);

			Q_UNUSED(childWidget);
		}
	}

	void SchemaViewWidget::deleteControlWidgets()
	{
		// Find all control items
		//
		std::map<QUuid, std::shared_ptr<VFrame30::SchemaItemControl>> controlItems;

		for (const auto& layer : schema()->layers())
		{
			for (const auto& item : layer->items())
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
		QObjectList childWidgets = children(); // Do not make childWidgets as a reference, as we change this list in the loop

		for (QObject* childObject : childWidgets)
		{
			QWidget* childWidget = dynamic_cast<QWidget*>(childObject);
			if (childWidget == nullptr)
			{
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
				// childWidget cannot just be deleted, as some events for this widget still can be in the message queue.
				// deleteLater() will delete object.
				//
				childWidget->setObjectName("");
				childWidget->hide();
				childWidget->deleteLater();
			}
		}

		return;
	}

	bool SchemaViewWidget::MousePosToDocPoint(const QPoint& mousePos, QPointF* pDestDocPos, double dpiX /*= 0*/, double dpiY /*= 0*/)
	{
		if (pDestDocPos == nullptr)
		{
			assert(pDestDocPos);
			return false;
		}

		double x = mousePos.x() * devicePixelRatioF();
		double y = mousePos.y() * devicePixelRatioF();

		if (schema()->unit() == SchemaUnit::Display)
		{
			pDestDocPos->setX(x / (m_zoom / 100.0));
			pDestDocPos->setY(y / (m_zoom / 100.0));
		}
		else
		{
			dpiX = dpiX == 0 ? realDpiX(this) : dpiX;
			dpiY = dpiY == 0 ? realDpiY(this) : dpiY;

			pDestDocPos->setX(x / (dpiX * (m_zoom / 100.0)));
			pDestDocPos->setY(y / (dpiY * (m_zoom / 100.0)));
		}

		return true;
	}

	void SchemaViewWidget::setSchema(std::shared_ptr<VFrame30::Schema> schema, bool repaint)
	{
		assert(schema.get() != nullptr);
		m_schema = schema;

		setZoom(zoom(), repaint); // Adjust sliders, widget etc.

		emit signal_schemaChanged(schema.get());
		return;
	}

	void SchemaViewWidget::draw(CDrawParam& drawParam, const QRectF& clipRect)
	{
		if (schema() == nullptr)
		{
			return;
		}

		updateControlWidgets(drawParam.drawMode() == DrawMode::Editor);

		// --
		//
		QPainter* p = drawParam.painter();

		// Calc size
		//
		// int widthInPixel = schema()->GetDocumentWidth(drawParam.realDpiX(), zoom());
		// int heightInPixel = schema()->GetDocumentHeight(drawParam.realDpiY(), zoom());

		// Clear device
		//
		// p->fillRect(QRectF(0, 0, widthInPixel + 1, heightInPixel + 1), QColor(0xB0, 0xB0, 0xB0));

		// Adjust QPainter.
		//
		Ajust(p, schema()->unit(), 0, 0, zoom());

		// Draw Schema
		//
		schema()->Draw(&drawParam, clipRect);

		return;
	}

	void SchemaViewWidget::paintEvent(QPaintEvent* /*paintEvent*/)
	{
		if (schema() == nullptr)
		{
			return;
		}

		QPainter p(this);
		CDrawParam drawParam(&p, this, schema()->gridSize(), schema()->pinGridStep(), schema()->unit());

		QRectF clipRect{0, 0, schema()->docWidth(), schema()->docHeight()};

		draw(drawParam, clipRect);

		return;
	}

	void SchemaViewWidget::mouseMoveEvent(QMouseEvent* event)
	{
		// If any control key is pressed, pass control further
		//
		if (event->buttons().testFlag(Qt::LeftButton) == true ||
			event->buttons().testFlag(Qt::RightButton) == true ||
			event->buttons().testFlag(Qt::MiddleButton) == true)
		{
			unsetCursor(); // set cursor to parent cursor

			event->ignore();
			return;
		}

		if (schema() == nullptr)
		{
			// Schema is not loaded
			//
			return;
		}

		// If the mouse cursor is over SchemaItem with acceptClick then set HandCursor
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

		for (const auto& layer : schema()->layers() | std::views::reverse)
		{
			if (layer->show() == false)
			{
				continue;
			}

			for (const auto& item : layer->items() | std::views::reverse)
			{
				if (item->acceptClick() == true &&
					item->isIntersectPoint(x, y) == true &&
					item->clickScript().isEmpty() == false)
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

	double SchemaViewWidget::setZoom(double value, bool repaint /*= true*/)
	{
		// Calc DPI
		//
		double realDpiX = physicalDpiX() * devicePixelRatioF();
		double realDpiY = physicalDpiY() * devicePixelRatioF();

		// if value is 0 then fit page into parent
		//
		if (value == 0)
		{
			QWidget* viewportWidget = this->parentWidget(); // Viewport can be real from QAbstractScrollArea or just any widget
			QAbstractScrollArea* abstractScrollArea = qobject_cast<QAbstractScrollArea*>(viewportWidget ? viewportWidget->parentWidget() : nullptr);

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
					horzScaleFactor = (viewportSize.width() * devicePixelRatioF()) / schema()->docWidth();
					vertScaleFactor = (viewportSize.height() * devicePixelRatioF()) / schema()->docHeight();
				}
				else
				{
					horzScaleFactor = (viewportSize.width() * devicePixelRatioF()) / (schema()->docWidth() * realDpiX);
					vertScaleFactor = (viewportSize.height() * devicePixelRatioF()) / (schema()->docHeight() * realDpiY);
				}

				value = std::min(vertScaleFactor, horzScaleFactor) * 100.0;
			}
		}
		else
		{
			value = qBound(30.0, value, 500.0);
		}

		m_zoom = value;

		// Width and height of the document in physical dpi's, taking into account devicePixelRatio
		// schema()->GetDocumentWidth() returns integer, for better precision we use double.
		// 
		//int widthInPixel = static_cast<int>(schema()->GetDocumentWidth(realDpiX, m_zoom));
		//int heightInPixel = static_cast<int>(schema()->GetDocumentHeight(realDpiY, m_zoom));

		double widthInPixel = schema()->unit() == SchemaUnit::Display ?
								  schema()->docWidth() * (m_zoom / 100.0) :
								  schema()->docWidth() * realDpiX * (m_zoom / 100.0);

		double heightInPixel = schema()->unit() == SchemaUnit::Display ?
								   schema()->docHeight() * (m_zoom / 100.0) :
								   schema()->docHeight() * realDpiY * (m_zoom / 100.0);

		// The size of window is set in different points
		// Qt widget points must be corrected according to devicePixelRatio()
		//
		int widthInQtPoints = std::round(widthInPixel / devicePixelRatio());
		int heightInQtPixel = std::round(heightInPixel / devicePixelRatio());

		QSize scaledPixelSize{widthInQtPoints, heightInQtPixel};

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
			this->update();
		}

		return m_zoom;
	}

} // namespace VFrame30
