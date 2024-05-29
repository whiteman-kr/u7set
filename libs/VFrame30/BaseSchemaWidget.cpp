#include <VFrame30/BaseSchemaWidget.h>
#include <VFrame30/SchemaView.h>
#include <VFrame30/Schema.h>


namespace VFrame30
{

	BaseSchemaWidget::BaseSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, SchemaViewWidget* schemaView, QWidget* parent) :
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
			int dx = event->pos().x() - m_mousePos.x();
			int dy = event->pos().y() - m_mousePos.y();

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
		double devicePixelRatio = m_schemaView->devicePixelRatioF();

		if (schema()->unit() == SchemaUnit::Display)
		{
			QPointF docPos = m_schemaView->mapFromParent(widgetPoint) * devicePixelRatio / (zoom() / 100.0);
			return docPos;
		}
		else
		{
			QPointF mappedToView = m_schemaView->mapFromParent(widgetPoint) * devicePixelRatio / (zoom() / 100.0);

			mappedToView.setX(mappedToView.x() / m_schemaView->realDpiX(m_schemaView));
			mappedToView.setY(mappedToView.y() / m_schemaView->realDpiY(m_schemaView));

			return mappedToView;
		}
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
		// If we have several screens with different resolution or DPI, this slot is required to recalculate
		// zoom params.
		//
		setZoom(zoom(), true);
	}

	VFrame30::Schema* BaseSchemaWidget::schema()
	{
		return m_schemaView->schema();
	}

	const VFrame30::Schema* BaseSchemaWidget::schema() const
	{
		return m_schemaView->schema();
	}

	std::shared_ptr<VFrame30::Schema> BaseSchemaWidget::schemaSharedPtr()
	{
		return m_schemaView->schemaSharedPtr();
	}

	std::shared_ptr<VFrame30::Schema> BaseSchemaWidget::schemaSharedPtr() const
	{
		return m_schemaView->schemaSharedPtr();
	}

	void BaseSchemaWidget::setSchema(std::shared_ptr<VFrame30::Schema> schema, bool repaint)
	{
		m_schemaView->setSchema(schema, repaint);
	}

	SchemaViewWidget* BaseSchemaWidget::schemaView()
	{
		return m_schemaView;
	}

	const SchemaViewWidget* BaseSchemaWidget::schemaView() const
	{
		return m_schemaView;
	}

	double BaseSchemaWidget::zoom() const
	{
		if (schemaView() == nullptr)
		{
			assert(schemaView() != nullptr);
			return 1.0;							// if return value is 0 then it is potential divide by 0
		}

		return schemaView()->zoom();
	}

	void BaseSchemaWidget::setZoom(double zoom, bool repaint, int horzScrollValue /*= -1*/, int vertScrollValue /*= -1*/)
	{
		QPoint widgetCenterPoint(viewport()->size().width() / 2, viewport()->size().height() / 2);
		QPointF oldDocPos =	widgetPointToDocument(widgetCenterPoint);

		zoom = schemaView()->setZoom(zoom, repaint);	// new zoom can be set

		QPointF newDocPos = widgetPointToDocument(widgetCenterPoint);
		QPointF dPos = (newDocPos - oldDocPos);							// Delta in document units

		switch (schema()->unit())
		{
		case SchemaUnit::Display:
			dPos.setX(dPos.x() * zoom / 100.0 / schemaView()->devicePixelRatioF());
			dPos.setY(dPos.y() * zoom / 100.0 / schemaView()->devicePixelRatioF());
			break;
		case SchemaUnit::Inch:
			dPos.setX(dPos.x() * schemaView()->realDpiX(schemaView()) * (zoom / 100.0) / schemaView()->devicePixelRatioF());
			dPos.setY(dPos.y() * schemaView()->realDpiY(schemaView()) * (zoom / 100.0) / schemaView()->devicePixelRatioF());
			break;
		default:
			assert(false);
		}

		int newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x());
		int newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y());

		horizontalScrollBar()->setValue(horzScrollValue == -1 ? newHorzValue : horzScrollValue);
		verticalScrollBar()->setValue(vertScrollValue == -1 ? newVertValue : vertScrollValue);

		return;
	}

}
