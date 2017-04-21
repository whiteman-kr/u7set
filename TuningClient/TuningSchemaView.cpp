#include "TuningSchemaView.h"
#include "TuningSchemaWidget.h"
#include "MainWindow.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/MonitorSchema.h"

TuningSchemaView::TuningSchemaView(SchemaStorage *schemaStorage, const QString& globalScript, QWidget *parent)
	:  SchemaView(parent),
	  m_schemaStorage(schemaStorage)
{
	setGlobalScript(globalScript);

	m_timer = new QTimer();
	connect(m_timer, &QTimer::timeout, this, &TuningSchemaView::onTimer);
	m_timer->start(250);
}

TuningSchemaView::~TuningSchemaView()
{
	delete m_timer;
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

	//qDebug()<<"paintEvent";

	QPainter p;
	p.begin(this);

	p.save();

	VFrame30::CDrawParam drawParam(&p, schema().get(), this, schema()->gridSize(), schema()->pinGridStep());
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

void TuningSchemaView::timerEvent(QTimerEvent*)
{

}


void TuningSchemaView::onTimer()
{
	update();
}


