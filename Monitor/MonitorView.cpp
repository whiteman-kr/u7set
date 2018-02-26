#include "MonitorView.h"
#include "MonitorSchemaManager.h"
#include "../lib/AppSignalManager.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/PropertyNames.h"


// MonitorView
//
MonitorView::MonitorView(MonitorSchemaManager* schemaManager, QWidget *parent)
	: VFrame30::ClientSchemaView(schemaManager, parent)
{
	qDebug() << Q_FUNC_INFO;
	return;
}

MonitorView::~MonitorView()
{
	qDebug() << Q_FUNC_INFO;
<<<<<<< HEAD
=======
}

void MonitorView::paintEvent(QPaintEvent* /*pe*/)
{
	// Draw Schema
	//
	//VFrame30::SchemaView::paintEvent(pe);

	QPainter p;
	p.begin(this);

	p.save();

	VFrame30::CDrawParam drawParam(&p, schema().get(), this, schema()->gridSize(), schema()->pinGridStep());

	drawParam.setBlinkPhase(static_cast<bool>((QTime::currentTime().msec() / 250) % 2));	// 0-249 : false, 250-499 : true, 500-749 : false, 750-999 : true
	drawParam.setEditMode(false);
	drawParam.setAppSignalManager(&theSignals);
	drawParam.setInfoMode(theSettings.showItemsLabels());

	// Draw schema
	//
	SchemaView::draw(drawParam);

	// Calc size
	//
	p.setRenderHint(QPainter::Antialiasing);

	// Ajust QPainter
	//
	Ajust(&p, 0, 0, zoom());

	p.restore();

	// --
	//
	p.end();

	return;
}

void MonitorView::timerEvent(QTimerEvent* /*event*/)
{
	// Guard timer in case if the main repaint timer has stopped
	//
	if (QDateTime::currentMSecsSinceEpoch() - m_lastRepaintEventFired.toMSecsSinceEpoch()  > 500_ms)
	{
		// Something wrong with timer, start it again
		//
		startRepaintTimer();
	}

	return;
}

void MonitorView::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons().testFlag(Qt::MidButton) == true)
	{
		// It is scrolling by midbutton, let scroll view process it
		//
		VFrame30::SchemaView::mousePressEvent(event);
		return;
	}

	// Find is there any item under the cursor with AcceptClick
	//
	m_leftClickOverItem.reset();

	QPointF docPoint;

	bool convertResult = MousePosToDocPoint(event->pos(), &docPoint);
	if (convertResult == false)
	{
		event->ignore();
		return;
	}

	double x = docPoint.x();
	double y = docPoint.y();

	for (auto layer = schema()->Layers.crbegin(); layer != schema()->Layers.crend(); layer++)
	{
		const VFrame30::SchemaLayer* pLayer = layer->get();

		if (pLayer->show() == false)
		{
			continue;
		}

		for (auto vi = pLayer->Items.crbegin(); vi != pLayer->Items.crend(); vi++)
		{
			const std::shared_ptr<VFrame30::SchemaItem>& item = *vi;

			if (item->acceptClick() == true && item->IsIntersectPoint(x, y) == true && item->clickScript().isEmpty() == false)
			{
				// Remember this item
				//
				m_leftClickOverItem = item;
				event->accept();
				return;
			}
		}
	}

	// Ignore event
	//
	event->ignore();
	return;
}

void MonitorView::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->buttons().testFlag(Qt::MidButton) == true)
	{
		// It is scrolling by midbutton, let scroll view process it
		//
		VFrame30::SchemaView::mouseReleaseEvent(event);
		return;
	}

	// Find is there any item under the cursor with AcceptClick
	//
	if (m_leftClickOverItem != nullptr)
	{
		QPointF docPoint;

		bool convertResult = MousePosToDocPoint(event->pos(), &docPoint);
		if (convertResult == false)
		{
			event->ignore();
			return;
		}

		double x = docPoint.x();
		double y = docPoint.y();

		for (auto layer = schema()->Layers.crbegin(); layer != schema()->Layers.crend(); layer++)
		{
			const VFrame30::SchemaLayer* pLayer = layer->get();

			if (pLayer->show() == false)
			{
				continue;
			}

			for (auto vi = pLayer->Items.crbegin(); vi != pLayer->Items.crend(); vi++)
			{
				const std::shared_ptr<VFrame30::SchemaItem>& item = *vi;

				if (item == m_leftClickOverItem &&
					item->acceptClick() == true &&
					item->clickScript().trimmed().isEmpty() == false &&
					item->IsIntersectPoint(x, y) == true &&
					item->clickScript().isEmpty() == false)
				{
					// Run script
					//
					item->clickEvent(globalScript(), jsEngine(), this);

					// --
					//
					update();		// Repaint screen
					unsetCursor();
					m_leftClickOverItem.reset();
					event->accept();
					return;
				}
			}
		}

		m_leftClickOverItem.reset();
	}

	// Ignore event
	//
	unsetCursor();
	event->ignore();
>>>>>>> develop
	return;
}




