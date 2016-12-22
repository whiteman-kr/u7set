#include "MonitorCentralWidget.h"
#include "SchemaManager.h"
#include "../VFrame30/MonitorSchema.h"
#include "../VFrame30/LogicSchema.h"

MonitorCentralWidget::MonitorCentralWidget(SchemaManager* schemaManager) :
	m_schemaManager(schemaManager)
{
	assert(m_schemaManager);

	// --
	//
	tabBar()->setExpanding(true);

	QSize sz = fontMetrics().size(Qt::TextSingleLine, "XEMPTYSCHEMAX");
	sz.setHeight(sz.height() * 1.75);

	setStyleSheet(QString("QTabBar::tab { min-width: %1px; min-height: %2px;})").arg(sz.width()).arg(sz.height()));

	// On start create an empty MonitorSchema and add a tab with this schema
	//
	addSchemaTabPage("EMPTYSCHEMA");

	// --
	//
	connect(this->tabBar(), &QTabBar::tabCloseRequested, this, &MonitorCentralWidget::slot_tabCloseRequested);
	connect(this, &MonitorCentralWidget::currentChanged, this, &MonitorCentralWidget::slot_tabPageChanged);

	connect(m_schemaManager, &SchemaManager::resetSchema, this, &MonitorCentralWidget::slot_resetSchema);

	return;
}

MonitorCentralWidget::~MonitorCentralWidget()
{
	qDebug() << Q_FUNC_INFO;
}

MonitorSchemaWidget* MonitorCentralWidget::currentTab()
{
	return dynamic_cast<MonitorSchemaWidget*>(currentWidget());
}

int MonitorCentralWidget::addSchemaTabPage(QString schemaId)
{
	std::shared_ptr<VFrame30::Schema> tabSchema = m_schemaManager->schema(schemaId);

	if (tabSchema == nullptr)
	{
		tabSchema = std::make_shared<VFrame30::MonitorSchema>();
		tabSchema->setSchemaId("EMPTYSCHEMA");
		tabSchema->setCaption("Empty Schema");
	}

	MonitorSchemaWidget* schemaWidget = new MonitorSchemaWidget(tabSchema, m_schemaManager);

	connect(schemaWidget, &MonitorSchemaWidget::signal_schemaChanged, this, &MonitorCentralWidget::slot_schemaChanged);
	connect(schemaWidget, &MonitorSchemaWidget::signal_historyChanged, this, &MonitorCentralWidget::signal_historyChanged);

	int index = addTab(schemaWidget, tabSchema->caption());

	if (count() > 1 && tabsClosable() == false)
	{
		setTabsClosable(true);
		setMovable(true);
	}

	emit signal_actionCloseTabUpdated(count() > 1);
	return index;
}

void MonitorCentralWidget::slot_newTab()
{
	MonitorSchemaWidget* curTabWidget = dynamic_cast<MonitorSchemaWidget*>(currentWidget());

	if (curTabWidget == nullptr)
	{
		assert(false);
		return;
	}

	slot_newSameTab(curTabWidget);
	return;
}

void MonitorCentralWidget::slot_closeCurrentTab()
{
	MonitorSchemaWidget* curTabWidget = dynamic_cast<MonitorSchemaWidget*>(currentWidget());

	if (curTabWidget == nullptr)
	{
		assert(false);
		return;
	}

	slot_closeTab(curTabWidget);

	curTabWidget->emitHistoryChanged();

	return;
}

void MonitorCentralWidget::slot_zoomIn()
{
	MonitorSchemaWidget* curTabWidget = dynamic_cast<MonitorSchemaWidget*>(currentWidget());

	if (curTabWidget == nullptr)
	{
		assert(false);
		return;
	}

	curTabWidget->zoomIn();
	return;
}

void MonitorCentralWidget::slot_zoomOut()
{
	MonitorSchemaWidget* curTabWidget = dynamic_cast<MonitorSchemaWidget*>(currentWidget());

	if (curTabWidget == nullptr)
	{
		assert(false);
		return;
	}

	curTabWidget->zoomOut();
	return;
}

void MonitorCentralWidget::slot_zoom100()
{
	MonitorSchemaWidget* curTabWidget = dynamic_cast<MonitorSchemaWidget*>(currentWidget());

	if (curTabWidget == nullptr)
	{
		assert(false);
		return;
	}

	curTabWidget->zoom100();
	return;
}

void MonitorCentralWidget::slot_historyBack()
{
	MonitorSchemaWidget* curTabWidget = dynamic_cast<MonitorSchemaWidget*>(currentWidget());

	if (curTabWidget == nullptr)
	{
		assert(false);
		return;
	}

	assert(curTabWidget->canBackHistory() == true);

	curTabWidget->historyBack();

	return;
}

void MonitorCentralWidget::slot_historyForward()
{
	MonitorSchemaWidget* curTabWidget = dynamic_cast<MonitorSchemaWidget*>(currentWidget());

	if (curTabWidget == nullptr)
	{
		assert(false);
		return;
	}

	assert(curTabWidget->canForwardHistory() == true);

	curTabWidget->historyForward();

	return;
}

void MonitorCentralWidget::slot_selectSchemaForCurrentTab(QString schemaId)
{
	MonitorSchemaWidget* tab = currentTab();

	if (tab == nullptr)
	{
		assert(tab);
		return;
	}

	tab->slot_setSchema(schemaId);

	tab->emitHistoryChanged();

	return;
}

void MonitorCentralWidget::slot_tabCloseRequested(int index)
{
	// Close Tab request
	//
	if (count() == 1)
	{
		// Don't close the last tab
		return;
	}

	MonitorSchemaWidget* tabWidget = dynamic_cast<MonitorSchemaWidget*>(widget(index));

	if (tabWidget == nullptr)
	{
		assert(tabWidget);
		return;
	}

	removeTab(index);
	delete tabWidget;

	if (count() <= 1)
	{
		setTabsClosable(false);
		setMovable(false);
	}

	emit signal_actionCloseTabUpdated(count() > 1);

	return;
}

void MonitorCentralWidget::slot_resetSchema(QString startSchemaId)
{
	// All schemas must be refreshed, apparently the new configuration has arrived
	// if there is no schema with prev SchemaID, load startSchemaId
	//
	for (int i = 0; i < count(); i++)
	{
		MonitorSchemaWidget* tabPage = dynamic_cast<MonitorSchemaWidget*>(widget(i));

		if (tabPage == nullptr)
		{
			assert(tabPage);
			continue;
		}

		tabPage->slot_setSchema(tabPage->schemaId());
		tabPage->resetHistory();

		if (i == currentIndex())
		{
			//emit signal_schemaChanged(newSchema->schemaID());
			emit signal_schemaChanged(tabPage->schema()->schemaId());
		}
	}

	return;
}

void MonitorCentralWidget::slot_newSameTab(MonitorSchemaWidget* tabWidget)
{
	if (tabWidget == nullptr)
	{
		assert(tabWidget);
		return;
	}

	QString schemaId = tabWidget->schema()->schemaId();
	int tabIndex = addSchemaTabPage(schemaId);

	// Switch to the new tab
	//
	if (tabIndex != -1)
	{
		setCurrentIndex(tabIndex);
		emit signal_schemaChanged(schemaId);

		MonitorSchemaWidget* newTab = currentTab();
		assert(newTab);

		newTab->emitHistoryChanged();
	}

	return;
}

void MonitorCentralWidget::slot_closeTab(MonitorSchemaWidget* tabWidget)
{
	if (tabWidget == nullptr)
	{
		assert(tabWidget);
		return;
	}

	int tabIndex = indexOf(tabWidget);

	if (tabIndex == -1)
	{
		assert(tabIndex != -1);
		return;
	}

	slot_tabCloseRequested(tabIndex);

	return;
}

void MonitorCentralWidget::slot_schemaChanged(MonitorSchemaWidget* tabWidget, VFrame30::Schema* schema)
{
	if (tabWidget == nullptr ||
		schema == nullptr)
	{
		assert(tabWidget);
		assert(schema);
		return;
	}

	int tabIndex = indexOf(tabWidget);

	if (tabIndex >=0)
	{
		setTabText(tabIndex, schema->caption());
	}

	emit signal_schemaChanged(schema->schemaId());
	tabWidget->emitHistoryChanged();

	return;
}

void MonitorCentralWidget::slot_tabPageChanged(int /*index*/)
{
	MonitorSchemaWidget* tab = currentTab();

	if (tab != nullptr)
	{
		emit signal_schemaChanged(tab->schemaId());
		tab->emitHistoryChanged();
	}

	return;
}



