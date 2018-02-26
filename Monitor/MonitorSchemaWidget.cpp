#include <QMessageBox>
#include "MonitorMainWindow.h"
#include "MonitorSchemaWidget.h"
#include "MonitorView.h"
#include "MonitorSchemaManager.h"
#include "DialogSignalInfo.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/MonitorSchema.h"

//
//
//	MonitorSchemaWidget
//
//

MonitorSchemaWidget::MonitorSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, MonitorSchemaManager* schemaManager) :
	VFrame30::ClientSchemaWidget(new MonitorView(schemaManager), schema, schemaManager)
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &MonitorSchemaWidget::contextMenuRequested);

	createActions();

	// --
	//
//	connect(schemaView(), &VFrame30::SchemaView::signal_schemaChanged, this, [this](VFrame30::Schema* schema)
//		{
//			emit this->signal_schemaChanged(this, schema);
//		});

	// --
	//
	connect(monitorSchemaView(), &MonitorView::signal_setSchema, this, &MonitorSchemaWidget::slot_setSchema);

//	// Init history
//	//
//	m_backHistory.push_back(currentHistoryState());

	return;
}

MonitorSchemaWidget::~MonitorSchemaWidget()
{
}

void MonitorSchemaWidget::createActions()
{
	// New tab (duplicate this one)
	//
//	m_newTabAction = new QAction(tr("New Tab"), this);
//	m_newTabAction->setEnabled(true);
//	QList<QKeySequence> newTabShortcuts;
//	newTabShortcuts << QKeySequence::AddTab;
//	newTabShortcuts << QKeySequence::New;
//	m_newTabAction->setShortcuts(newTabShortcuts);
//	//m_newTabAction->setShortcutContext(Qt::WidgetShortcut);		// To avoid abigious with main menu same QAction
//	addAction(m_newTabAction);

//	connect(m_newTabAction, &QAction::triggered, this, [this](){ emit signal_newTab(this);});

//	// Closet current tab
//	//
//	m_closeTabAction = new QAction(tr("Close Tab"), this);
//	m_closeTabAction->setEnabled(true);
//	//m_closeTabAction->setShortcuts(QKeySequence::Close);			// To avoid abigious with main menu same QAction
//	addAction(m_closeTabAction);

//	connect(m_closeTabAction, &QAction::triggered, this, [this](){ emit signal_closeTab(this);});

	// --
	//

}

//void MonitorSchemaWidget::mousePressEvent(QMouseEvent* event)
//{
//	if (event->buttons().testFlag(Qt::LeftButton) == true)
//	{
//		m_dragStartPosition = event->pos();
//		VFrame30::BaseSchemaWidget::mousePressEvent(event);
//	}
//	else
//	{
//		VFrame30::BaseSchemaWidget::mousePressEvent(event);
//	}
//}

//void MonitorSchemaWidget::mouseMoveEvent(QMouseEvent* event)
//{
//	if (event->buttons().testFlag(Qt::LeftButton) == false)
//	{
//		VFrame30::BaseSchemaWidget::mouseMoveEvent(event);
//		return;
//	}

//	// Left button is pressed, is this start of drag and drop
//	//
//	if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
//	{
//		VFrame30::BaseSchemaWidget::mouseMoveEvent(event);
//		return;
//	}

//	// Try to start drag and drop
//	//
//	QPointF docPos = widgetPointToDocument(m_dragStartPosition);

//	std::shared_ptr<VFrame30::SchemaItem> schemaItem;

//	for (auto layer : schema()->Layers)
//	{
//		if (layer->compile() == false)
//		{
//			continue;
//		}

//		schemaItem = layer->getItemUnderPoint(docPos, QLatin1String("VFrame30::SchemaItemInput"));
//		if (schemaItem != nullptr)
//		{
//			break;
//		}

//		schemaItem = layer->getItemUnderPoint(docPos, QLatin1String("VFrame30::SchemaItemOutput"));
//		if (schemaItem != nullptr)
//		{
//			break;
//		}

//		schemaItem = layer->getItemUnderPoint(docPos, QLatin1String("VFrame30::SchemaItemInOut"));
//		break;
//	}

//	if (schemaItem == nullptr)
//	{
//		return;
//	}

//	std::shared_ptr<VFrame30::SchemaItemSignal> schemaItemSignal = std::dynamic_pointer_cast<VFrame30::SchemaItemSignal>(schemaItem);
//	if (schemaItemSignal == nullptr)
//	{
//		assert(schemaItemSignal);
//		return;
//	}

//	// Save signals to protobufer
//	//
//	::Proto::AppSignalSet protoSetMessage;
//	QStringList appSignalIds = schemaItemSignal->appSignalIdList();

//	for (QString id : appSignalIds)
//	{
//		bool ok = false;
//		AppSignalParam signalParam = theSignals.signalParam(id, &ok);
//		if (ok == false)
//		{
//			continue;
//		}

//		assert(signalParam.appSignalId() == id);

//		::Proto::AppSignal* protoSignalMessage = protoSetMessage.add_appsignal();
//		signalParam.save(protoSignalMessage);
//	}

//	if (protoSetMessage.appsignal_size() == 0)
//	{
//		return;
//	}

//	QByteArray data;
//	data.resize(protoSetMessage.ByteSize());

//	protoSetMessage.SerializeToArray(data.data(), protoSetMessage.ByteSize());

//	// --
//	//
//	if (data.isEmpty() == false)
//	{
//		QDrag* drag = new QDrag(this);
//		QMimeData* mimeData = new QMimeData;

//		mimeData->setData(AppSignalParamMimeType::value, data);
//		drag->setMimeData(mimeData);

//		drag->exec(Qt::CopyAction);

//		qDebug() << "Start drag for " << appSignalIds;
//		qDebug() << "Drag and drop data buffer size " << data.size();
//	}

//	return;
//}


//std::vector<std::shared_ptr<VFrame30::SchemaItem>> MonitorSchemaWidget::itemsUnderCursor(const QPoint& pos)
//{
//	std::vector<std::shared_ptr<VFrame30::SchemaItem>> result;
//	result.reserve(8);

//	QPointF docPoint;

//	bool convertResult = MousePosToDocPoint(pos, &docPoint);
//	if (convertResult == false)
//	{
//		return result;
//	}

//	double x = docPoint.x();
//	double y = docPoint.y();

//	for (auto layer = schema()->Layers.crbegin(); layer != schema()->Layers.crend(); layer++)
//	{
//		const VFrame30::SchemaLayer* pLayer = layer->get();

//		if (pLayer->show() == false)
//		{
//			continue;
//		}

//		for (auto vi = pLayer->Items.crbegin(); vi != pLayer->Items.crend(); vi++)
//		{
//			const std::shared_ptr<VFrame30::SchemaItem>& item = *vi;

//			if (item->IsIntersectPoint(x, y) == true)
//			{
//				result.push_back(item);
//			}
//		}
//	}

//	return result;
//}

//bool MonitorSchemaWidget::canBackHistory() const
//{
//	bool enableBack = m_backHistory.size() > 1;
//	return enableBack;
//}

//bool MonitorSchemaWidget::canForwardHistory() const
//{
//	bool enableForward = !m_forwardHistory.empty();
//	return enableForward;
//}

//void MonitorSchemaWidget::historyBack()
//{
//	qDebug() << "MonitorSchemaWidget::historyBack()";

//	if (m_backHistory.empty() == true)
//	{
//		return;
//	}

//	SchemaHistoryItem currentView = m_backHistory.back();
//	m_backHistory.pop_back();

//	assert(currentView.m_schemaId == schemaId());	// Save current state
//	currentView = currentHistoryState();

//	m_forwardHistory.push_front(currentView);

//	if (m_backHistory.empty() == true)
//	{
//		return;
//	}

//	SchemaHistoryItem& restoreItem = m_backHistory.back();
//	restoreState(restoreItem);

//	emitHistoryChanged();

//	return;
//}

//void MonitorSchemaWidget::historyForward()
//{
//	qDebug() << "MonitorSchemaWidget::historyForward()";

//	if (m_forwardHistory.empty() == true)
//	{
//		return;
//	}

//	// save current state
//	//
//	SchemaHistoryItem& currentView = m_backHistory.back();
//	assert(currentView.m_schemaId == schemaId());
//	currentView = currentHistoryState();

//	// switch history
//	//
//	SchemaHistoryItem hi = m_forwardHistory.front();
//	m_forwardHistory.pop_front();

//	m_backHistory.push_back(hi);

//	restoreState(hi);

//	emitHistoryChanged();

//	return;
//}

//void MonitorSchemaWidget::resetHistory()
//{
//	qDebug() << "MonitorSchemaWidget::resetHistory()";

//	m_backHistory.clear();
//	m_backHistory.push_back(currentHistoryState());

//	m_forwardHistory.clear();

//	emitHistoryChanged();

//	return;
//}

//void MonitorSchemaWidget::restoreState(const SchemaHistoryItem& historyState)
//{
//	if (m_schemaManager == nullptr)
//	{
//		assert(m_schemaManager);
//		return;
//	}

//	std::shared_ptr<VFrame30::Schema> schema = m_schemaManager->schema(historyState.m_schemaId);

//	if (schema == nullptr)
//	{
//		// and there is no startSchemaId (((
//		// Just create an empty schema
//		//
//		schema = std::make_shared<VFrame30::MonitorSchema>();
//		schema->setSchemaId(historyState.m_schemaId);
//		schema->setCaption(historyState.m_schemaId + " not found");
//	}

//	// --
//	//
//	setSchema(schema, false);
//	setZoom(historyState.m_zoom, false);

//	horizontalScrollBar()->setValue(historyState.m_horzScrollValue);
//	verticalScrollBar()->setValue(historyState.m_vertScrollValue);

//	schemaView()->repaint();

//	return;
//}

//SchemaHistoryItem MonitorSchemaWidget::currentHistoryState() const
//{
//	SchemaHistoryItem hi;

//	hi.m_schemaId = schemaId();
//	hi.m_zoom = zoom();
//	hi.m_horzScrollValue = horizontalScrollBar()->value();
//	hi.m_vertScrollValue = verticalScrollBar()->value();

//	return hi;
//}

//void MonitorSchemaWidget::emitHistoryChanged()
//{
//	emit signal_historyChanged(canBackHistory(), canForwardHistory());
//	return;
//}

void MonitorSchemaWidget::contextMenuRequested(const QPoint& pos)
{
	// Disable/enable actions
	//

	//m_fileSaveAction->setEnabled(readOnly() == false && modified() == true);


	// Signals items
	//
	std::vector<std::shared_ptr<VFrame30::SchemaItem>> items = itemsUnderCursor(pos);

	if (items.empty() == false)
	{
		for (const std::shared_ptr<VFrame30::SchemaItem>& item : items)
		{
			VFrame30::SchemaItemSignal* schemaItemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get());

			if (schemaItemSignal != nullptr)
			{
				const QStringList& signalList = schemaItemSignal->appSignalIdList();

				signalContextMenu(signalList);
				break;
			}

			VFrame30::SchemaItemReceiver* schemaItemReceiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get());

			if (schemaItemReceiver != nullptr)
			{
				QStringList signalList;
				QString s = schemaItemReceiver->appSignalId();
				signalList.push_back(s);

				signalContextMenu(signalList);
				break;
			}

		}
	}

//
//	actions << m_newTabAction;
//	actions << m_closeTabAction;

	return;
}

void MonitorSchemaWidget::signalContextMenu(const QStringList signalList)
{
	// Compose menu
	//
	QMenu menu(this);
	QList<QAction*> actions;

	for (const QString& s : signalList)
	{
		bool ok = false;
		AppSignalParam signal =	theSignals.signalParam(s, &ok);

		QString signalId = ok ? QString("%1 %2").arg(signal.customSignalId()).arg(signal.caption()) : s;

		QAction* a = new QAction(signalId, &menu);

		auto f = [this, s]() -> void
				 {
					signalInfo(s);
				 };

		connect(a, &QAction::triggered, this, f);

		actions << a;
	}

	menu.exec(actions, QCursor::pos(), 0, this);
}

void MonitorSchemaWidget::signalInfo(QString appSignalId)
{
	if (theMonitorMainWindow == nullptr)
	{
		assert(theMonitorMainWindow);
		return;
	}

	bool ok = false;
	AppSignalParam signal = theSignals.signalParam(appSignalId, &ok);

	if (ok == true)
	{
		DialogSignalInfo* dsi = new DialogSignalInfo(signal, theMonitorMainWindow);
		dsi->show();
	}
	else
	{
		QMessageBox::critical(this, qAppName(), tr("Signal %1 not found.").arg(appSignalId));
	}

	return;
}

void MonitorSchemaWidget::slot_setSchema(QString schemaId)
{
	if (schemaManager() == nullptr)
	{
		assert(schemaManager());
		return;
	}

	// Save current state to the history
	//
	resetForwardHistory();

	if (canBackHistory() == false)
	{
		VFrame30::SchemaHistoryItem& currentHistoryItem = m_backHistory.back();
		assert(currentHistoryItem.m_schemaId == this->schemaId());

		currentHistoryItem = currentHistoryState();
	}

	// --
	//
	std::shared_ptr<VFrame30::Schema> schema = schemaManager()->schema(schemaId);

	if (schema == nullptr)
	{
		// and there is no startSchemaId (((
		// Just create an empty schema
		//
		schema = std::make_shared<VFrame30::MonitorSchema>();
		schema->setSchemaId("EMPTYSCHEMA");
		schema->setCaption("Empty Schema");
	}

	// --
	//
	setSchema(schema, false);
	setZoom(100.0, true);

	// --
	//
	VFrame30::SchemaHistoryItem hi = currentHistoryState();
	m_backHistory.push_back(hi);

	// --
	//
	emitHistoryChanged();

	return;
}

//QString MonitorSchemaWidget::schemaId() const
//{
//	if (schema() == nullptr)
//	{
//		return QString();
//	}

//	return schema()->schemaId();
//}

//QString MonitorSchemaWidget::caption() const
//{
//	if (schema() == nullptr)
//	{
//		return QString();
//	}

//	return schema()->caption();
//}

MonitorView* MonitorSchemaWidget::monitorSchemaView()
{
	MonitorView* result = dynamic_cast<MonitorView*>(schemaView());
	assert(result);
	return result;
}

const MonitorView* MonitorSchemaWidget::monitorSchemaView() const
{
	const MonitorView* result = dynamic_cast<const MonitorView*>(schemaView());
	assert(result);
	return result;
}
