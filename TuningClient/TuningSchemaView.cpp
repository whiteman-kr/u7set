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

	startRepaintTimer();	// This is a main repaint timer, it firse on the edge of 250ms
	startTimer(1000);		// This is a guard timer
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

	//qDebug() << "paintEvent";

	QPainter p;
	p.begin(this);

	p.save();

	VFrame30::CDrawParam drawParam(&p, schema().get(), this, schema()->gridSize(), schema()->pinGridStep());

	drawParam.setTuningController(&tuningController());
	drawParam.setBlinkPhase(static_cast<bool>((QTime::currentTime().msec() / 250) % 2));	// 0-249 : false, 250-499 : true, 500-749 : false, 750-999 : true
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
	// Guard timer in case if the main repaint timer has stopped
	//
	if (QDateTime::currentMSecsSinceEpoch() - m_lastRepaintEventFired.toMSecsSinceEpoch()  > 500)
	{
		// Something wrong with timer, start it again
		//
		startRepaintTimer();
	}
}

void TuningSchemaView::startRepaintTimer()
{
	update();

	// Set this timer in the edge of 250ms
	//
	int64_t currentMs = QTime::currentTime().msec();
	int64_t ms = (currentMs / 250 + 1) * 250 - currentMs;

	QTimer::singleShot(ms, this, &TuningSchemaView::startRepaintTimer);
	m_lastRepaintEventFired = QDateTime::currentDateTime();

	return;
}

