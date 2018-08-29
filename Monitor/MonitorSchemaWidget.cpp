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

MonitorSchemaWidget::MonitorSchemaWidget(std::shared_ptr<VFrame30::Schema> schema,
										 MonitorSchemaManager* schemaManager,
										 VFrame30::AppSignalController* appSignalController,
										 VFrame30::TuningController* tuningController) :
	VFrame30::ClientSchemaWidget(new MonitorView(schemaManager), schema, schemaManager)
{
	clientSchemaView()->setAppSignalController(appSignalController);
	clientSchemaView()->setTuningController(tuningController);

	// --
	//
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &MonitorSchemaWidget::contextMenuRequested);

	createActions();

	// --
	//
	connect(monitorSchemaView(), &MonitorView::signal_setSchema, this, &VFrame30::ClientSchemaWidget::setSchema);

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

//void MonitorSchemaWidget::slot_setSchema(QString schemaId)
//{
//	if (schemaManager() == nullptr)
//	{
//		assert(schemaManager());
//		return;
//	}

//	// Save current state to the history
//	//
//	resetForwardHistory();

//	if (canBackHistory() == false)
//	{
//		VFrame30::SchemaHistoryItem& currentHistoryItem = m_backHistory.back();
//		assert(currentHistoryItem.m_schemaId == this->schemaId());

//		currentHistoryItem = currentHistoryState();
//	}

//	// --
//	//
//	std::shared_ptr<VFrame30::Schema> schema = schemaManager()->schema(schemaId);

//	if (schema == nullptr)
//	{
//		// and there is no startSchemaId (((
//		// Just create an empty schema
//		//
//		schema = std::make_shared<VFrame30::MonitorSchema>();
//		schema->setSchemaId("EMPTYSCHEMA");
//		schema->setCaption("Empty Schema");
//	}

//	// --
//	//
//	setSchema(schema, false);
//	setZoom(100.0, true);

//	// --
//	//
//	VFrame30::SchemaHistoryItem hi = currentHistoryState();
//	m_backHistory.push_back(hi);

//	// --
//	//
//	emitHistoryChanged();

//	return;
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
