#include "SimSchemaWidget.h"
#include "SimSchemaView.h"
#include "SimSchemaManager.h"
#include "../VFrame30/SchemaItemSignal.h"
#include "../VFrame30/SchemaItemConnection.h"
#include "../VFrame30/AppSignalController.h"

//
//
//	SimSchemaWidget
//
//

SimSchemaWidget::SimSchemaWidget(std::shared_ptr<VFrame30::Schema> schema,
								 SimSchemaManager* schemaManager,
								 VFrame30::AppSignalController* appSignalController,
								 VFrame30::TuningController* tuningController) :
	VFrame30::ClientSchemaWidget(new SimSchemaView(schemaManager), schema, schemaManager)
{
	clientSchemaView()->setAppSignalController(appSignalController);
	clientSchemaView()->setTuningController(tuningController);

	setFrameShape(QFrame::NoFrame);

	// --
	//
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &SimSchemaWidget::contextMenuRequested);

	createActions();

	// --
	//
	bool vynesti_etu_huinyu_iz_konstructora_v_bazovyi_class;
	connect(simSchemaView(), &SimSchemaView::signal_setSchema, this, &VFrame30::ClientSchemaWidget::setSchema);

	return;
}

SimSchemaWidget::~SimSchemaWidget()
{
}

void SimSchemaWidget::createActions()
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

void SimSchemaWidget::contextMenuRequested(const QPoint& pos)
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

void SimSchemaWidget::signalContextMenu(const QStringList signalList)
{
	// Compose menu
	//
	QMenu menu(this);
	QList<QAction*> actions;

	VFrame30::AppSignalController* appSignalController = simSchemaView()->appSignalController();
	if (appSignalController == nullptr)
	{
		assert(appSignalController);
		return;
	}

	for (const QString& s : signalList)
	{
		bool ok = false;
		AppSignalParam signal =	appSignalController->signalParam(s, &ok);

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

void SimSchemaWidget::signalInfo(QString appSignalId)
{
	bool to_do_show_signal_info_dialog;

//	if (theMonitorMainWindow == nullptr)
//	{
//		assert(theMonitorMainWindow);
//		return;
//	}

//	bool ok = false;
//	AppSignalParam signal = theSignals.signalParam(appSignalId, &ok);

//	if (ok == true)
//	{
//		DialogSignalInfo* dsi = new DialogSignalInfo(signal, theMonitorMainWindow);
//		dsi->show();
//	}
//	else
//	{
//		QMessageBox::critical(this, qAppName(), tr("Signal %1 not found.").arg(appSignalId));
//	}

//	return;
}

SimSchemaView* SimSchemaWidget::simSchemaView()
{
	SimSchemaView* result = dynamic_cast<SimSchemaView*>(schemaView());
	assert(result);
	return result;
}

const SimSchemaView* SimSchemaWidget::simSchemaView() const
{
	const SimSchemaView* result = dynamic_cast<const SimSchemaView*>(schemaView());
	assert(result);
	return result;
}
