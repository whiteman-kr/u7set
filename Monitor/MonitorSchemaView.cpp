#include "MonitorSchemaView.h"
#include "SchemaManager.h"
#include "../lib/AppSignalManager.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/PropertyNames.h"

MonitorSchemaView::MonitorSchemaView(SchemaManager* schemaManager, QWidget *parent)
	: SchemaView(parent),
	  m_schemaManager(schemaManager)
{
	qDebug() << Q_FUNC_INFO;
	assert(m_schemaManager);

	startTimer(250);

	return;
}

MonitorSchemaView::~MonitorSchemaView()
{
	qDebug() << Q_FUNC_INFO;
}

void MonitorSchemaView::paintEvent(QPaintEvent* /*pe*/)
{
	// Draw Schema
	//
	//VFrame30::SchemaView::paintEvent(pe);

	QPainter p;
	p.begin(this);

	p.save();

	VFrame30::CDrawParam drawParam(&p, schema().get(), this, schema()->gridSize(), schema()->pinGridStep());
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

void MonitorSchemaView::timerEvent(QTimerEvent* /*event*/)
{
	update();
}

void MonitorSchemaView::mousePressEvent(QMouseEvent* event)
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

void MonitorSchemaView::mouseReleaseEvent(QMouseEvent* event)
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
	return;
}

void MonitorSchemaView::setSchema(QString schemaId)
{
	// We can't change schema here, because we need to save history, so emit signal and change schema
	// in MonitorSchemaWidget
	//
	emit signal_setSchema(schemaId);

	return;
}

QString MonitorSchemaView::globalScript() const
{
	if (m_schemaManager == nullptr)
	{
		assert(m_schemaManager);
		return QString();
	}

	return m_schemaManager->globalScript();
}

