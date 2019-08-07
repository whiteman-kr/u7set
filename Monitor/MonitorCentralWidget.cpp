#include "MonitorCentralWidget.h"
#include "MonitorSchemaManager.h"
#include "../VFrame30/MonitorSchema.h"
#include "../VFrame30/LogicSchema.h"

MonitorCentralWidget::MonitorCentralWidget(MonitorSchemaManager* schemaManager,
										   VFrame30::AppSignalController* appSignalController,
										   VFrame30::TuningController* tuningController) :
	m_schemaManager(schemaManager),
	m_appSignalController(appSignalController),
	m_tuningController(tuningController)
{
	Q_ASSERT(m_schemaManager);

	// --
	//
	tabBar()->setExpanding(true);

	QSize sz = fontMetrics().size(Qt::TextSingleLine, "XEMPTYSCHEMAX");
	sz.setHeight(static_cast<int>(sz.height() * 1.75));

	setStyleSheet(QString("QTabBar::tab { min-width: %1px; min-height: %2px;})").arg(sz.width()).arg(sz.height()));

	// On start create an empty MonitorSchema and add a tab with this schema
	//
	addSchemaTabPage("EMPTYSCHEMA", {});

	// --
	//
	connect(this->tabBar(), &QTabBar::tabCloseRequested, this, &MonitorCentralWidget::slot_tabCloseRequested);
	connect(this, &MonitorCentralWidget::currentChanged, this, &MonitorCentralWidget::slot_tabPageChanged);

	connect(m_schemaManager, &VFrame30::SchemaManager::schemasWereReseted, this, &MonitorCentralWidget::slot_resetSchema);

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

VFrame30::TuningController* MonitorCentralWidget::tuningController()
{
	return m_tuningController;
}

int MonitorCentralWidget::addSchemaTabPage(QString schemaId, const QVariantHash& variables)
{
	std::shared_ptr<VFrame30::Schema> tabSchema = m_schemaManager->schema(schemaId);

	if (tabSchema == nullptr)
	{
		tabSchema = std::make_shared<VFrame30::MonitorSchema>();
		tabSchema->setSchemaId("EMPTYSCHEMA");
		tabSchema->setCaption("Empty Schema");
	}

	MonitorSchemaWidget* schemaWidget = new MonitorSchemaWidget(tabSchema,
																m_schemaManager,
																m_appSignalController,
																m_tuningController);
	schemaWidget->clientSchemaView()->setVariables(variables);

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
		Q_ASSERT(curTabWidget);
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
		Q_ASSERT(curTabWidget);
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
		Q_ASSERT(curTabWidget);
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
		Q_ASSERT(curTabWidget);
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
		Q_ASSERT(curTabWidget);
		return;
	}

	curTabWidget->zoom100();
	return;
}

void MonitorCentralWidget::slot_historyBack()
{
	MonitorSchemaWidget* curTabWidget = dynamic_cast<MonitorSchemaWidget*>(currentWidget());

	if (curTabWidget == nullptr || curTabWidget->canBackHistory() == false)
	{
		Q_ASSERT(curTabWidget);
		Q_ASSERT(curTabWidget->canBackHistory() == true);
		return;
	}

	curTabWidget->historyBack();

	return;
}

void MonitorCentralWidget::slot_historyForward()
{
	MonitorSchemaWidget* curTabWidget = dynamic_cast<MonitorSchemaWidget*>(currentWidget());

	if (curTabWidget == nullptr || curTabWidget->canForwardHistory() == false)
	{
		Q_ASSERT(false);
		Q_ASSERT(curTabWidget->canForwardHistory());
		return;
	}

	curTabWidget->historyForward();

	return;
}

void MonitorCentralWidget::slot_selectSchemaForCurrentTab(QString schemaId)
{
	MonitorSchemaWidget* tab = currentTab();

	if (tab == nullptr)
	{
		Q_ASSERT(tab);
		return;
	}

	tab->setSchema(schemaId);

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
		Q_ASSERT(tabWidget);
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

void MonitorCentralWidget::slot_resetSchema()
{
	// All schemas must be refreshed, apparently the new configuration has arrived
	// if there is no schema with prev SchemaID, load startSchemaId
	//
	for (int i = 0; i < count(); i++)
	{
		MonitorSchemaWidget* tabPage = dynamic_cast<MonitorSchemaWidget*>(widget(i));
		if (tabPage == nullptr)
		{
			Q_ASSERT(tabPage);
			continue;
		}

		QString schemaToLoad = tabPage->schemaId();
		if (m_schemaManager->hasSchema(tabPage->schemaId()) == true)
		{
			schemaToLoad = tabPage->schemaId();
		}
		else
		{
			QString startSchemaId = m_schemaManager->monitorConfigController()->configurationStartSchemaId();
			if (m_schemaManager->hasSchema(startSchemaId) == true)
			{
				schemaToLoad = startSchemaId;
			}
			else
			{
				// schemaToLoad will stay tabPage->schemaId(); as during initialization,
				// in that case empty schema will be loaded
				//
			}
		}

		tabPage->setSchema(schemaToLoad);
		tabPage->resetHistory();

		if (i == currentIndex())
		{
			emit signal_schemaChanged(tabPage->schema()->schemaId());
		}
	}

	return;
}

void MonitorCentralWidget::slot_newSameTab(MonitorSchemaWidget* tabWidget)
{
	if (tabWidget == nullptr)
	{
		Q_ASSERT(tabWidget);
		return;
	}

	QString schemaId = tabWidget->schema()->schemaId();
	QVariantHash variables = tabWidget->clientSchemaView()->variables();

	int tabIndex = addSchemaTabPage(schemaId, variables);

	// Switch to the new tab
	//
	if (tabIndex != -1)
	{
		setCurrentIndex(tabIndex);
		emit signal_schemaChanged(schemaId);

		MonitorSchemaWidget* newTab = currentTab();
		Q_ASSERT(newTab);

		newTab->emitHistoryChanged();
	}

	return;
}

void MonitorCentralWidget::slot_closeTab(MonitorSchemaWidget* tabWidget)
{
	if (tabWidget == nullptr)
	{
		Q_ASSERT(tabWidget);
		return;
	}

	int tabIndex = indexOf(tabWidget);

	if (tabIndex == -1)
	{
		Q_ASSERT(tabIndex != -1);
		return;
	}

	slot_tabCloseRequested(tabIndex);
	return;
}

void MonitorCentralWidget::slot_schemaChanged(VFrame30::ClientSchemaWidget* tabWidget, VFrame30::Schema* schema)
{
	if (tabWidget == nullptr ||
		schema == nullptr)
	{
		Q_ASSERT(tabWidget);
		Q_ASSERT(schema);
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



