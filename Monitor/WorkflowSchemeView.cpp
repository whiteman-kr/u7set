#include "WorkflowSchemeView.h"

WorkflowSchemeView::WorkflowSchemeView(QWidget *parent)
	: SchemeView(parent)
{
	qDebug() << Q_FUNC_INFO;
}

WorkflowSchemeView::~WorkflowSchemeView()
{
	qDebug() << Q_FUNC_INFO;
}

void WorkflowSchemeView::paintEvent(QPaintEvent* pe)
{
	// Draw Scheme
	//
	VFrame30::SchemeView::paintEvent(pe);

	QPainter p;
	p.begin(this);

	p.save();

	VFrame30::CDrawParam drawParam(&p, scheme().get(), scheme()->gridSize(), scheme()->pinGridStep());

	// Calc size
	//
	p.setRenderHint(QPainter::Antialiasing);

	// Ajust QPainter
	//
	Ajust(&p, 0, 0, zoom());

	// Draw Scheme
	//
	QRectF clipRect(0, 0, scheme()->docWidth(), scheme()->docHeight());

	// Items are being moved drawing
	//
	//drawMovingItems(&drawParam);

	// --
	//
	//drawRectSizing(&drawParam);
	//drawMovingLinePoint(&drawParam);
	//drawMovingEdgesOrVertexConnectionLine(&drawParam);

	p.restore();

	// --
	//
	p.end();

	return;
}
