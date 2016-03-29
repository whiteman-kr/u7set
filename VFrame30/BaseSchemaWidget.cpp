#include "BaseSchemaWidget.h"
#include "Schema.h"
#include "SchemaView.h"

namespace VFrame30
{

	BaseSchemaWidget::BaseSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, SchemaView* schemaView)
	{
		assert(schema != nullptr);
		assert(schemaView != nullptr);

		setBackgroundRole(QPalette::Dark);
		setAlignment(Qt::AlignCenter);
		setMouseTracking(true);

		// --
		//
		m_schemaView = schemaView;
		m_schemaView->setSchema(schema, false);

		m_schemaView->setZoom(100);
		setWidget(m_schemaView);

		// --
		//
		createActions();

		return;
	}

	BaseSchemaWidget::~BaseSchemaWidget()
	{

	}

	void BaseSchemaWidget::createActions()
	{
		// View->ZoomIn
		//
		m_zoomInAction = new QAction(tr("Zoom In"), this);
		m_zoomInAction->setEnabled(true);
		m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
		m_zoomInAction->setShortcutContext(Qt::ShortcutContext::WindowShortcut);
		connect(m_zoomInAction, &QAction::triggered, this, &BaseSchemaWidget::zoomIn);
		addAction(m_zoomInAction);

		// View->ZoomOut
		//
		m_zoomOutAction = new QAction(tr("Zoom Out"), this);
		m_zoomOutAction->setEnabled(true);
		m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
		connect(m_zoomOutAction, &QAction::triggered, this, &BaseSchemaWidget::zoomOut);
		addAction(m_zoomOutAction);

		// View->Zoom100
		//
		m_zoom100Action = new QAction(tr("Zoom 100%"), this);
		m_zoom100Action->setEnabled(true);
		m_zoom100Action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Asterisk));
		connect(m_zoom100Action, &QAction::triggered, this, &BaseSchemaWidget::zoom100);
		addAction(m_zoom100Action);
	}

	void BaseSchemaWidget::wheelEvent(QWheelEvent* event)
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
			double zoom = schemaView()->zoom() + numSteps * 10;

			QPointF oldDocPos;
			MousePosToDocPoint(event->pos(), &oldDocPos);

			schemaView()->setZoom(zoom, false);

			QPointF newDocPos;
			MousePosToDocPoint(event->pos(), &newDocPos);

			//
			//
			QPointF dPos = (newDocPos - oldDocPos);

			int newHorzValue = 0;
			int newVertValue = 0;

			switch (schema()->unit())
			{
			case VFrame30::SchemaUnit::Display:
				newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * zoom / 100.0);
				newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * zoom / 100.0);
				break;
			case VFrame30::SchemaUnit::Inch:
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

	void BaseSchemaWidget::mousePressEvent(QMouseEvent* event)
	{
		if (event->button() == Qt::MidButton)
		{
			// Enter to scrolling mode
			//
			m_mousePos = event->pos();

			m_horzScrollBarValue = horizontalScrollBar()->value();
			m_vertScrollBarValue = verticalScrollBar()->value();

			setCursor(Qt::OpenHandCursor);

			event->accept();
			return;
		}

		// Event was not accepted
		//
		event->setAccepted(false);
		return;
	}

	void BaseSchemaWidget::mouseMoveEvent(QMouseEvent* event)
	{
		if (event->buttons().testFlag(Qt::MidButton) == true)
		{
			int dx = event->x() - m_mousePos.x();
			int dy = event->y() - m_mousePos.y();

			horizontalScrollBar()->setValue(m_horzScrollBarValue - dx);
			verticalScrollBar()->setValue(m_vertScrollBarValue - dy);

			event->accept();
			return;
		}

		// Event was not accepted
		//
		event->setAccepted(false);
		return;
	}

	QPointF BaseSchemaWidget::widgetPointToDocument(const QPoint& widgetPoint) const
	{
		double docX = 0;	// Result
		double docY = 0;

		double dpiX = logicalDpiX();
		double dpiY = logicalDpiY();

		int widthInPixels = schema()->GetDocumentWidth(dpiX, zoom());
		int heightInPixels = schema()->GetDocumentHeight(dpiY, zoom());

		QRect clientRect = geometry();

		int startX = 0;
		int startY = 0;

		if (clientRect.width() >= widthInPixels)
		{
			startX = (clientRect.width() - widthInPixels) / 2;
		}
		else
		{
			startX = -horizontalScrollBar()->value();
		}

		if (clientRect.height() >= heightInPixels)
		{
			startY = (clientRect.height() - heightInPixels) / 2;
		}
		else
		{
			startY = -verticalScrollBar()->value();
		}

		double x = widgetPoint.x() - startX;		// position in points
		double y = widgetPoint.y() - startY;

		// Scaling to zoom factor
		//
		if (schema()->unit() == VFrame30::SchemaUnit::Display)
		{
			docX = x / (zoom() / 100.0);
			docY = y / (zoom() / 100.0);
		}
		else
		{
			docX = x / (dpiX * (zoom() / 100.0));
			docY = y / (dpiY * (zoom() / 100.0));
		}
		
		return QPointF(docX, docY);
	}

	bool BaseSchemaWidget::MousePosToDocPoint(const QPoint& mousePos, QPointF* destDocPos, int dpiX /*= 0*/, int dpiY /*= 0*/)
	{
		if (destDocPos == nullptr)
		{
			assert(destDocPos != nullptr);
			return false;
		}

		dpiX = dpiX == 0 ? logicalDpiX() : dpiX;
		dpiY = dpiY == 0 ? logicalDpiY() : dpiY;

		double zoom = schemaView()->zoom();

		int widthInPixels = schema()->GetDocumentWidth(dpiX, zoom);
		int heightInPixels = schema()->GetDocumentHeight(dpiY, zoom);

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

		if (schema()->unit() == VFrame30::SchemaUnit::Display)
		{
			destDocPos->setX(x / (zoom / 100.0));
			destDocPos->setY(y / (zoom / 100.0));
		}
		else
		{
			destDocPos->setX(x / (dpiX * (zoom / 100.0)));
			destDocPos->setY(y / (dpiY * (zoom / 100.0)));
		}

		return true;
	}

	void BaseSchemaWidget::zoomIn()
	{
		setZoom(zoom() + 10);
		return;
	}

	void BaseSchemaWidget::zoomOut()
	{
		setZoom(zoom() - 10);
		return;
	}

	void BaseSchemaWidget::zoom100()
	{
		setZoom(100);
		return;
	}

	std::shared_ptr<VFrame30::Schema> BaseSchemaWidget::schema()
	{
		return m_schemaView->schema();
	}

	const std::shared_ptr<VFrame30::Schema> BaseSchemaWidget::schema() const
	{
		return m_schemaView->schema();
	}

	void BaseSchemaWidget::setSchema(std::shared_ptr<VFrame30::Schema> schema)
	{
		m_schemaView->setSchema(schema, true);
	}

	SchemaView* BaseSchemaWidget::schemaView()
	{
		return m_schemaView;
	}

	const SchemaView* BaseSchemaWidget::schemaView() const
	{
		return m_schemaView;
	}

	double BaseSchemaWidget::zoom() const
	{
		if (schemaView() == nullptr)
		{
			assert(schemaView() != nullptr);
			return 0;
		}

		return schemaView()->zoom();
	}

	void BaseSchemaWidget::setZoom(double zoom, int horzScrollValue /*= -1*/, int vertScrollValue /*= -1*/)
	{
		QPoint widgetCenterPoint(size().width() / 2, size().height() / 2);

		QPointF oldDocPos;
		MousePosToDocPoint(widgetCenterPoint, &oldDocPos);

		schemaView()->setZoom(zoom, false);

		QPointF newDocPos;
		MousePosToDocPoint(widgetCenterPoint, &newDocPos);

		// --
		//
		QPointF dPos = (newDocPos - oldDocPos);

		// --
		//
		int newHorzValue = 0;
		int newVertValue = 0;

		switch (schema()->unit())
		{
		case VFrame30::SchemaUnit::Display:
			newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * zoom / 100.0);
			newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * zoom / 100.0);
			break;
		case VFrame30::SchemaUnit::Inch:
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
