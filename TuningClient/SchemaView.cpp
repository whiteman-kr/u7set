#include "SchemaView.h"
#include "SchemaWidget.h"
#include "MainWindow.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/MonitorSchema.h"

TuningSchemaView::TuningSchemaView(SchemaStorage *schemaStorage, QWidget *parent)
	:  SchemaView(parent),
	  m_schemaStorage(schemaStorage)
{
}

TuningSchemaView::~TuningSchemaView()
{
}

void TuningSchemaView::setSchema(QString schemaId)
{
	emit signal_setSchema(schemaId);
}

void TuningSchemaView::paintEvent(QPaintEvent* /*pe*/)
{
	// Draw Schema
	//
	//VFrame30::SchemaView::paintEvent(pe);

	QPainter p;
	p.begin(this);

	p.save();

	VFrame30::CDrawParam drawParam(&p, schema().get(), schema()->gridSize(), schema()->pinGridStep());
	drawParam.setEditMode(false);

	// Draw schema
	//
	SchemaView::draw(drawParam);

	// Calc size
	//
	p.setRenderHint(QPainter::Antialiasing);

	// Ajust QPainter
	//
	Ajust(&p, 0, 0, zoom());

	// Draw Schema
	//
	QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());


	p.restore();

	// --
	//
	p.end();

	return;
}


void TuningSchemaView::timerEvent(QTimerEvent* /*event*/)
{
}

