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

void MonitorSchemaView::runScript(const QString& script, VFrame30::SchemaItem* schemaItem)
{
	qDebug() << "MonitorSchemaView::runScript";

	if (script.isEmpty() == true ||
		schemaItem == nullptr)
	{
		assert(schemaItem);
		return;
	}

	QJSValue jsSchemaItem = m_jsEngine.newQObject(schemaItem);;
	QQmlEngine::setObjectOwnership(schemaItem, QQmlEngine::CppOwnership);

	// Run script
	//
	QJSValue jsEval = m_jsEngine.evaluate(script, "VFrame30::SchemaItem::ClickScript");
	if (jsEval.isError() == true)
	{
		QMessageBox::critical(this, QApplication::applicationDisplayName(), tr("Script error: %1").arg(jsEval.toString()));
		return;
	}

	QJSValueList args;
	args << jsSchemaItem;

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


void MonitorSchemaView::mouseMoveEvent(QMouseEvent* event)
{
	if (schema() == nullptr)
	{
		// Schema is not loaded
		//
		event->ignore();
		return;
	}

	if (event->buttons().testFlag(Qt::MidButton) == true)
	{
		// It is scrolling by midbutton, let scroll view process it
		//
		event->ignore();
		return;
	}

	// If the mouse cursor is over SchmeItem with acceptClick then set HandCursor
	//
	QPointF docPoint;

	bool convertResult = MousePosToDocPoint(event->pos(), &docPoint);
	if (convertResult == false)
	{
		unsetCursor();
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
				setCursor(Qt::PointingHandCursor);
				event->accept();
				return;
			}
		}
	}

	// --
	//
	unsetCursor();
	event->ignore();
	return;
}

void MonitorSchemaView::mousePressEvent(QMouseEvent* event)
{
	qDebug() << "MonitorSchemaView::mousePressEvent";

	if (event->buttons().testFlag(Qt::MidButton) == true)
	{
		// It is scrolling by midbutton, let scroll view process it
		//
		event->ignore();
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
	qDebug() << "MonitorSchemaView::mouseReleaseEvent";

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
	event->ignore();
	return;
}
