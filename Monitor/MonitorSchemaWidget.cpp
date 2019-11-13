#include "MonitorMainWindow.h"
#include "MonitorSchemaWidget.h"
#include "MonitorView.h"
#include "MonitorSchemaManager.h"
#include "DialogSignalInfo.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemValue.h"
#include "../VFrame30/SchemaItemImageValue.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/MonitorSchema.h"
#include "../VFrame30/MacrosExpander.h"

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
	qDebug() << Q_FUNC_INFO;
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
	std::vector<SchemaItemPtr> items = itemsUnderCursor(pos);

	if (items.empty() == false)
	{
		for (const SchemaItemPtr& item : items)
		{
			if (VFrame30::SchemaItemSignal* schemaItemSignal = dynamic_cast<VFrame30::SchemaItemSignal*>(item.get());
				schemaItemSignal != nullptr)
			{
				QStringList signalList = schemaItemSignal->appSignalIdList() + schemaItemSignal->impactAppSignalIdList();
				signalContextMenu(signalList);
				break;
			}

			if (VFrame30::SchemaItemValue* schemaItem = dynamic_cast<VFrame30::SchemaItemValue*>(item.get());
				schemaItem != nullptr)
			{
				QStringList signalList = VFrame30::MacrosExpander::parse(schemaItem->signalIds(), clientSchemaView(), &clientSchemaView()->session(), schema().get(), schemaItem);
				signalContextMenu(signalList);
				break;
			}

			if (VFrame30::SchemaItemImageValue* schemaItem = dynamic_cast<VFrame30::SchemaItemImageValue*>(item.get());
				schemaItem != nullptr)
			{
				QStringList signalList = VFrame30::MacrosExpander::parse(schemaItem->signalIds(), clientSchemaView(), &clientSchemaView()->session(), schema().get(), schemaItem);
				signalContextMenu(signalList);
				break;
			}

			if (VFrame30::SchemaItemReceiver* schemaItemReceiver = dynamic_cast<VFrame30::SchemaItemReceiver*>(item.get());
				schemaItemReceiver != nullptr)
			{
				signalContextMenu(schemaItemReceiver->appSignalIdsAsList());
				break;
			}

		}
	}

//
//	actions << m_newTabAction;
//	actions << m_closeTabAction;

	return;
}

void MonitorSchemaWidget::signalContextMenu(const QStringList& signalList)
{
	qDebug() << "signalContextMenu: " << signalList;

	// To set, it will sort list and exclude same ids
	//
	std::set<QString> signalListSet;
	for (const QString& s : signalList)
	{
		signalListSet.insert(s);
	}

	// Compose menu
	//
	QMenu menu(this);

	// Schemas List
	//
	QMenu* schemasSubMenu = menu.addMenu(tr("Schemas"));

	std::set<QString> signalsSchemasSet;
	for (const QString& s : signalListSet)
	{
		QStringList schemaIds = schemaManager()->monitorConfigController()->schemasByAppSignalId(s);

		for (const QString& schemaId : schemaIds)
		{
			signalsSchemasSet.insert(schemaId);
		}
	}

	if (signalsSchemasSet.empty() == true)
	{
		schemasSubMenu->setDisabled(true);
	}
	else
	{
		for (const QString& schemaId : signalsSchemasSet)
		{
			auto f = [this, schemaId, signalList]() -> void
					 {
						if (schemaId != this->schemaId())
						{
							setSchema(schemaId, signalList);
						}
					 };

			QString actionCaption = (schema()->schemaId() == schemaId) ? QString("-> %1").arg(schemaId) : schemaId;

			QAction* a = schemasSubMenu->addAction(actionCaption);
			connect(a, &QAction::triggered, this, f);
		}
	}

	// SignalInfo list
	//
	menu.addSeparator();

	for (const QString& s : signalListSet)
	{
		bool ok = false;
		AppSignalParam signal =	theSignals.signalParam(s, &ok);

		QString signalId = ok ? QString("%1 %2").arg(signal.customSignalId()).arg(signal.caption()) : s;

		QAction* a = menu.addAction(signalId);

		auto f = [this, s]() -> void
				 {
					signalInfo(s);
				 };

		connect(a, &QAction::triggered, this, f);
	}

	// --
	//
	menu.exec(QCursor::pos());

	return;
}

void MonitorSchemaWidget::signalInfo(QString appSignalId)
{
	DialogSignalInfo::showDialog(appSignalId, theMonitorMainWindow->configController(), theMonitorMainWindow->monitorCentralWidget());

	return;
}

//void MonitorSchemaWidget::slot_setSchema(QString schemaId)
//{
//	if (schemaManager() == nullptr)
//	{
//		Q_ASSERT(schemaManager());
//		return;
//	}

//	// Save current state to the history
//	//
//	resetForwardHistory();

//	if (canBackHistory() == false)
//	{
//		VFrame30::SchemaHistoryItem& currentHistoryItem = m_backHistory.back();
//		Q_ASSERT(currentHistoryItem.m_schemaId == this->schemaId());

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
	Q_ASSERT(result);
	return result;
}

const MonitorView* MonitorSchemaWidget::monitorSchemaView() const
{
	const MonitorView* result = dynamic_cast<const MonitorView*>(schemaView());
	Q_ASSERT(result);
	return result;
}

MonitorSchemaManager* MonitorSchemaWidget::schemaManager()
{
	return monitorSchemaView()->monitorSchemaManager();
}

const MonitorSchemaManager* MonitorSchemaWidget::schemaManager() const
{
	return monitorSchemaView()->monitorSchemaManager();
}
