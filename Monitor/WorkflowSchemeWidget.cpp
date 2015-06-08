#include "WorkflowSchemeWidget.h"
#include "WorkflowSchemeView.h"


//
//
//	WorkflowSchemeWidget
//
//

WorkflowSchemeWidget::WorkflowSchemeWidget(std::shared_ptr<VFrame30::Scheme> scheme)
{
	createActions();

	setBackgroundRole(QPalette::Dark);
	setAlignment(Qt::AlignCenter);
	setMouseTracking(true);

	// --
	//
	m_schemeView = new WorkflowSchemeView(this);
	m_schemeView->setScheme(scheme, false);

	m_schemeView->setZoom(100);
	setWidget(m_schemeView);

	// --
	//
	//connect(this, &QWidget::customContextMenuRequested, this, &WorkflowSchemeWidget::contextMenu);

	return;
}

WorkflowSchemeWidget::~WorkflowSchemeWidget()
{
}

void WorkflowSchemeWidget::createActions()
{

}


QPointF WorkflowSchemeWidget::widgetPointToDocument(const QPoint& widgetPoint) const
{
	double docX = 0;	// Result
	double docY = 0;

	double dpiX = logicalDpiX();
	double dpiY = logicalDpiY();

	int widthInPixels = scheme()->GetDocumentWidth(dpiX, zoom());
	int heightInPixels = scheme()->GetDocumentHeight(dpiY, zoom());

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
	if (scheme()->unit() == VFrame30::SchemeUnit::Display)
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

bool WorkflowSchemeWidget::MousePosToDocPoint(const QPoint& mousePos, QPointF* destDocPos, int dpiX /*= 0*/, int dpiY /*= 0*/)
{
	if (destDocPos == nullptr)
	{
		assert(destDocPos != nullptr);
		return false;
	}

	dpiX = dpiX == 0 ? logicalDpiX() : dpiX;
	dpiY = dpiY == 0 ? logicalDpiY() : dpiY;

	double zoom = schemeView()->zoom();

	int widthInPixels = scheme()->GetDocumentWidth(dpiX, zoom);
	int heightInPixels = scheme()->GetDocumentHeight(dpiY, zoom);

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

	if (scheme()->unit() == VFrame30::SchemeUnit::Display)
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

void WorkflowSchemeWidget::zoomIn()
{
	setZoom(zoom() + 10);
	return;
}

void WorkflowSchemeWidget::zoomOut()
{
	setZoom(zoom() - 10);
	return;
}

void WorkflowSchemeWidget::zoom100()
{
	setZoom(100);
	return;
}

std::shared_ptr<VFrame30::Scheme> WorkflowSchemeWidget::scheme()
{
	return m_schemeView->scheme();
}

const std::shared_ptr<VFrame30::Scheme> WorkflowSchemeWidget::scheme() const
{
	return m_schemeView->scheme();
}

void WorkflowSchemeWidget::setScheme(std::shared_ptr<VFrame30::Scheme> scheme)
{
	m_schemeView->setScheme(scheme, true);
}

WorkflowSchemeView* WorkflowSchemeWidget::schemeView()
{
	return m_schemeView;
}

const WorkflowSchemeView* WorkflowSchemeWidget::schemeView() const
{
	return m_schemeView;
}

double WorkflowSchemeWidget::zoom() const
{
	if (schemeView() == nullptr)
	{
		assert(schemeView() != nullptr);
		return 0;
	}

	return schemeView()->zoom();
}

void WorkflowSchemeWidget::setZoom(double zoom, int horzScrollValue /*= -1*/, int vertScrollValue /*= -1*/)
{
	QPoint widgetCenterPoint(size().width() / 2, size().height() / 2);

	QPointF oldDocPos;
	MousePosToDocPoint(widgetCenterPoint, &oldDocPos);

	schemeView()->setZoom(zoom, false);

	QPointF newDocPos;
	MousePosToDocPoint(widgetCenterPoint, &newDocPos);

	// --
	//
	QPointF dPos = (newDocPos - oldDocPos);

	// --
	//
	int newHorzValue = 0;
	int newVertValue = 0;

	switch (scheme()->unit())
	{
	case VFrame30::SchemeUnit::Display:
		newHorzValue = horizontalScrollBar()->value() - static_cast<int>(dPos.x() * zoom / 100.0);
		newVertValue = verticalScrollBar()->value() - static_cast<int>(dPos.y() * zoom / 100.0);
		break;
	case VFrame30::SchemeUnit::Inch:
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
