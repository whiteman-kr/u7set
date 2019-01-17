#include "BaseSchemaWidget.h"
#include "SchemaView.h"
#include "Schema.h"
#include <QWindow>

namespace VFrame30
{

	BaseSchemaWidget::BaseSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, SchemaView* schemaView)
	{
		assert(schema != nullptr);
		assert(schemaView != nullptr);

		setBackgroundRole(QPalette::Window);
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

		// Create a widget, make it invisible and set Qt::WA_NativeWindow to it, then
		// connect slot to it QWindow::screenChanged.
		// The problem was, to get QWindow from widget it must be native window, as I don't want to make
		// BaseSchemaWidget native, I have created this small invisible wiget
		//
		QWidget* dummyNativeWidget = new QWidget(this);
		dummyNativeWidget->setAttribute(Qt::WA_NativeWindow, true);

		if (dummyNativeWidget->windowHandle() != nullptr)
		{
			connect(dummyNativeWidget->windowHandle(), &QWindow::screenChanged, this, &BaseSchemaWidget::screenChanged);
		}
		else
		{
			assert(dummyNativeWidget->windowHandle());
		}

		return;
	}

	BaseSchemaWidget::~BaseSchemaWidget()
	{

	}

	void BaseSchemaWidget::createActions()
	{
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
			setZoom(zoom, false);
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

	void BaseSchemaWidget::mouseReleaseEvent(QMouseEvent* event)
	{
		if (event->button() == Qt::MidButton)
		{
			// Leave scrolling mode;
			//
			unsetCursor();
			event->accept();
			return;
		}

		event->ignore();
		return;
	}

	void BaseSchemaWidget::mouseMoveEvent(QMouseEvent* event)
	{
		if (event->buttons().testFlag(Qt::MidButton) == true)
		{
			// Scrolling mode
			//
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

		double dpiX = physicalDpiX();
		double dpiY = physicalDpiY();

		int widthInPixels = schema()->GetDocumentWidth(dpiX, zoom());
		int heightInPixels = schema()->GetDocumentHeight(dpiY, zoom());

		QRect clientRect = geometry();

		if (horizontalScrollBar() != nullptr &&
			horizontalScrollBar()->isVisible())
		{
			clientRect.setHeight(clientRect.height() - horizontalScrollBar()->height());
		}

		if (verticalScrollBar() != nullptr &&
			verticalScrollBar()->isVisible())
		{
			clientRect.setWidth(clientRect.width() - verticalScrollBar()->width());
		}

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

		dpiX = dpiX == 0 ? physicalDpiX() : dpiX;
		dpiY = dpiY == 0 ? physicalDpiY() : dpiY;

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
		setZoom(zoom() + 20, false);
		return;
	}

	void BaseSchemaWidget::zoomOut()
	{
		setZoom(zoom() - 20, false);
		return;
	}

	void BaseSchemaWidget::zoom100()
	{
		setZoom(100, false);
		return;
	}

	void BaseSchemaWidget::screenChanged(QScreen* /*screen*/)
	{
		// If we have several screens with different resolution or DPI, this slot is requeired to recalculate
		// zoom params
		//
		setZoom(zoom(), true);
	}

	std::shared_ptr<VFrame30::Schema> BaseSchemaWidget::schema()
	{
		return m_schemaView->schema();
	}

	const std::shared_ptr<VFrame30::Schema> BaseSchemaWidget::schema() const
	{
		return m_schemaView->schema();
	}

	void BaseSchemaWidget::setSchema(std::shared_ptr<VFrame30::Schema> schema, bool repaint)
	{
		m_schemaView->setSchema(schema, repaint);
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

	void BaseSchemaWidget::setZoom(double zoom, bool repaint, int horzScrollValue /*= -1*/, int vertScrollValue /*= -1*/)
	{
		QPoint widgetCenterPoint(size().width() / 2, size().height() / 2);

		QPointF oldDocPos;
		MousePosToDocPoint(widgetCenterPoint, &oldDocPos);

		schemaView()->setZoom(zoom, repaint);

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
			newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * (zoom / 100.0) * physicalDpiX());
			newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * (zoom / 100.0) * physicalDpiY());
			break;
		default:
			assert(false);
		}

		horizontalScrollBar()->setValue(horzScrollValue == -1 ? newHorzValue : horzScrollValue);
		verticalScrollBar()->setValue(vertScrollValue == -1 ? newVertValue : vertScrollValue);

		return;
	}

}
