#include "MonitorSchemaView.h"
#include "SchemaManager.h"
#include "../include/AppSignalManager.h"

MonitorSchemaView::MonitorSchemaView(SchemaManager* schemaManager, QWidget *parent)
	: SchemaView(parent),
	  m_schemaManager(schemaManager)
{
	qDebug() << Q_FUNC_INFO;
	assert(m_schemaManager);

	return;
}

MonitorSchemaView::~MonitorSchemaView()
{
	qDebug() << Q_FUNC_INFO;
}

void MonitorSchemaView::runScript(const QString& script, VFrame30::SchemaItem* schemaItem)
{
	qDebug() << "MonitorSchemaView::runScript";

	if (script.isEmpty() == true ||
		schemaItem == nullptr)
	{
		assert(schemaItem);
		return;
	}

	// Evaluate script
	//
	QJSValue jsEval = m_jsEngine.evaluate(script, "VFrame30::SchemaItem::ClickScript");
	if (jsEval.isError() == true)
	{
		QMessageBox::critical(this, QApplication::applicationDisplayName(), tr("Script error: %1").arg(jsEval.toString()));
		return;
	}

	// Create JS params
	//
	QJSValue jsSchemaItem = m_jsEngine.newQObject(schemaItem);;
	QQmlEngine::setObjectOwnership(schemaItem, QQmlEngine::CppOwnership);

	QJSValue jsSchemaView = m_jsEngine.newQObject(this);
	QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

	// Set argument list
	//
	QJSValueList args;

	args << jsSchemaItem;
	args << jsSchemaView;

	// Run script
	//
	QJSValue jsResult = jsEval.call(args);
	if (jsResult.isError() == true)
	{
		qDebug() << "Uncaught exception at line"
				 << jsResult.property("lineNumber").toInt()
				 << ":" << jsResult.toString();

		QMessageBox::critical(this, QApplication::applicationDisplayName(), tr("Script uncaught exception: %1").arg(jsEval.toString()));
		return;
	}

	qDebug() << "runScript result:" <<  jsResult.toInt();

	m_jsEngine.collectGarbage();

	update();		// Repaint screen
	return;
}

bool MonitorSchemaView::setSchema(QString schemaId)
{
	assert(m_schemaManager);

	std::shared_ptr<VFrame30::Schema> schema = m_schemaManager->schema(schemaId);

	if (schema == nullptr)
	{
		return false;
	}

	SchemaView::setSchema(schema, false);
	setZoom(100.0, true);

	return true;
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

	drawParam.setEditMode(false);
	drawParam.setAppSignalManager(&theSignals);


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
					item->IsIntersectPoint(x, y) == true &&
					item->clickScript().isEmpty() == false)
				{
					// Run script
					//
					runScript(item->clickScript(), item.get());

					// --
					//
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
