#include "MonitorSchemaView.h"

MonitorSchemaView::MonitorSchemaView(QWidget *parent)
	: SchemaView(parent)
{
	qDebug() << Q_FUNC_INFO;
}

MonitorSchemaView::~MonitorSchemaView()
{
	qDebug() << Q_FUNC_INFO;
}

void MonitorSchemaView::paintEvent(QPaintEvent* pe)
{
	// Draw Schema
	//
	VFrame30::SchemaView::paintEvent(pe);

	QPainter p;
	p.begin(this);

	p.save();

	VFrame30::CDrawParam drawParam(&p, schema().get(), schema()->gridSize(), schema()->pinGridStep());

	// Calc size
	//
	p.setRenderHint(QPainter::Antialiasing);

	// Ajust QPainter
	//
	Ajust(&p, 0, 0, zoom());

	// Draw Schema
	//
	QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());

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
