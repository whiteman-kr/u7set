#include "MonitorSchemaWidget.h"
#include "MonitorSchemaView.h"
#include "SchemaManager.h"

//
//
//	MonitorSchemaWidget
//
//

MonitorSchemaWidget::MonitorSchemaWidget(std::shared_ptr<VFrame30::Schema> schema, SchemaManager* schemaManager) :
	BaseSchemaWidget(schema, new MonitorSchemaView(schemaManager)),
	m_schemaManager(schemaManager)
{
	assert(m_schemaManager);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &MonitorSchemaWidget::contextMenu);

	createActions();

	connect(schemaView(), &VFrame30::SchemaView::signal_schemaChanged, this, [this](VFrame30::Schema* schema)
		{
			emit this->signal_schemaChanged(this, schema);
		});

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

QString MonitorSchemaWidget::schemaId() const
{
	if (schema() == nullptr)
	{
		return QString();
	}

	return schema()->schemaID();
}

QString MonitorSchemaWidget::caption() const
{
	if (schema() == nullptr)
	{
		return QString();
	}

	return schema()->caption();
}

void MonitorSchemaWidget::contextMenu(const QPoint& /*pos*/)
{
	// Disable/enable actions
	//

	//m_fileSaveAction->setEnabled(readOnly() == false && modified() == true);

	// Compose menu
	//
	QMenu menu(this);

//	QList<QAction*> actions;

//	actions << m_newTabAction;
//	actions << m_closeTabAction;

//	menu.exec(actions, mapToGlobal(pos), 0, this);
	return;
}



