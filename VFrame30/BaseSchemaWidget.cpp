#include "BaseSchemaWidget.h"
#include "SchemaView.h"
#include "Schema.h"


namespace VFrame30
{

	BaseSchemaWidget::BaseSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, SchemaView* schemaView, QWidget* parent) :
		QScrollArea(parent),
		m_schemaView(schemaView)
	{
		setFrameStyle(QFrame::NoFrame);

		assert(schema != nullptr);
		assert(schemaView != nullptr);

		setBackgroundRole(QPalette::Window);
		setAlignment(Qt::AlignCenter);
		setMouseTracking(true);

		// --
		//
		m_schemaView->setSchemaInternal(schema);

		setWidget(m_schemaView);

		// --
		//
		createActions();

		// Create a widget, make it invisible and set Qt::WA_NativeWindow to it, then
		// connect slot to it QWindow::screenChanged.
		// The problem was, to get QWindow from widget it must be native window, as I don't want to make
		// BaseSchemaWidget native, I have created this small invisible wiget
		//
		// UPDATE: Unfortuanatelly this native window makes flicking on moving slider of docking widget with schema
		//         list while. So it is commented ((
		//

//		QWidget* dummyNativeWidget = new QWidget(this);
//		dummyNativeWidget->setAttribute(Qt::WA_NativeWindow, true);

//		if (dummyNativeWidget->windowHandle() != nullptr)
//		{
//			connect(dummyNativeWidget->windowHandle(), &QWindow::screenChanged, this, &BaseSchemaWidget::screenChanged);
//		}
//		else
//		{
//			assert(dummyNativeWidget->windowHandle());
//		}

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
		if (event->buttons().testFlag(Qt::MiddleButton))
		{
			return;
		}

        int numDegrees = event->angleDelta().y() / 8;
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
		if (event->button() == Qt::MiddleButton)
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
		if (event->button() == Qt::MiddleButton)
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
		if (event->buttons().testFlag(Qt::MiddleButton) == true)
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

		double dpiX = logicalDpiX();
		double dpiY = logicalDpiY();

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

		const double x = widgetPoint.x() - startX;		// position in points
		const double y = widgetPoint.y() - startY;

		// Scaling to zoom factor
		//
		if (schema()->unit() == SchemaUnit::Display)
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

		const double zoom = schemaView()->zoom();

		int widthInPixels = schema()->GetDocumentWidth(dpiX, zoom);
		int heightInPixels = schema()->GetDocumentHeight(dpiY, zoom);

		int startX = 0;
		int startY = 0;

		QRect widgetRect = viewport()->rect();

		if (widgetRect.width() >= widthInPixels)
		{
			startX = (widgetRect.width() - widthInPixels) / 2;
		}
		else
		{
			startX = -horizontalScrollBar()->value();
		}

		if (widgetRect.height() >= heightInPixels)
		{
			startY = (widgetRect.height() - heightInPixels) / 2;
		}
		else
		{
			startY = -verticalScrollBar()->value();
		}

		const int x = mousePos.x() - startX;
		const int y = mousePos.y() - startY;

		if (schema()->unit() == SchemaUnit::Display)
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
		double value = (static_cast<int>(zoom()) / static_cast<int>(ZoomStep)) * static_cast<int>(ZoomStep);
		setZoom(value + ZoomStep, false);

		return;
	}

	void BaseSchemaWidget::zoomOut()
	{
		double value = (static_cast<int>(zoom()) / static_cast<int>(ZoomStep)) * static_cast<int>(ZoomStep);
		setZoom(value - ZoomStep, false);

		return;
	}

	void BaseSchemaWidget::zoom100()
	{
		setZoom(100, false);
		return;
	}

	void BaseSchemaWidget::zoomToFit()
	{
		setZoom(0, false);
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
			return 1.0;							// if return value 0 then it is potential divide by 0
		}

		return schemaView()->zoom();
	}

	void BaseSchemaWidget::setZoom(double zoom, bool repaint, int horzScrollValue /*= -1*/, int vertScrollValue /*= -1*/)
	{
		QPoint widgetCenterPoint(viewport()->size().width() / 2,
								 viewport()->size().height() / 2);

		QPointF oldDocPos;
		MousePosToDocPoint(widgetCenterPoint, &oldDocPos);

		zoom = schemaView()->setZoom(zoom, repaint);	// new zoom can be set

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
		case SchemaUnit::Display:
			newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * zoom / 100.0);
			newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * zoom / 100.0);
			break;
		case SchemaUnit::Inch:
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
