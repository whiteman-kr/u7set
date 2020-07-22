#include "MonitorCentralWidget.h"
#include "MonitorSchemaManager.h"
#include "../VFrame30/MonitorSchema.h"
#include "../VFrame30/LogicSchema.h"
#include "../lib/Ui/SchemaListWidget.h"

MonitorCentralWidget::MonitorCentralWidget(MonitorSchemaManager* schemaManager,
										   VFrame30::AppSignalController* appSignalController,
										   VFrame30::TuningController* tuningController,
										   QWidget* parent) :
	TabWidgetEx(parent),
	m_schemaManager(schemaManager),
	m_appSignalController(appSignalController),
	m_tuningController(tuningController)
{
	Q_ASSERT(m_schemaManager);

	// --
	//
	tabBar()->setExpanding(true);

	// At first we see just one tab, so it is not closable
	//
	setTabsClosable(false);
	setMovable(false);

	// --
	//
	connect(this->tabBar(), &QTabBar::tabCloseRequested, this, &MonitorCentralWidget::slot_tabCloseRequested);
	connect(this, &MonitorCentralWidget::currentChanged, this, &MonitorCentralWidget::slot_tabPageChanged);

	connect(m_schemaManager, &VFrame30::SchemaManager::schemasWereReseted, this, &MonitorCentralWidget::slot_resetSchema);

	m_eventLoopTimerId = startTimer(1);

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

void MonitorCentralWidget::timerEvent(QTimerEvent* event)
{
	if (m_eventLoopTimerId != 0)
	{
		m_eventLoopTimerCounter ++;

		if (event->timerId() == m_eventLoopTimerId && m_eventLoopTimerCounter > 10)
		{
			killTimer(m_eventLoopTimerId);
			m_eventLoopTimerId = 0;

			// Create first tab here
			// Problem - we set zoom to FitToScreen, for it we need to have window geometry
			// but in constructor it is not set correctly, and it changes when message loop starts/
			// So we have to process several messages in message loop and only after that
			// we can add new tab page
			//
			slot_newSchemaTab("EMPTY");
		}
	}

	return;
}

int MonitorCentralWidget::addSchemaTabPage(QString schemaId, const QVariantHash& variables)
{
	std::shared_ptr<VFrame30::Schema> tabSchema = m_schemaManager->schema(schemaId);

	if (tabSchema == nullptr)
	{
		// If schema is not fount try to set StartSchemaID
		//
		tabSchema = m_schemaManager->schema(m_schemaManager->monitorConfigController()->configurationStartSchemaId());

		if (tabSchema == nullptr)
		{
			tabSchema = std::make_shared<VFrame30::MonitorSchema>();
			tabSchema->setSchemaId("EMPTYSCHEMA");
			tabSchema->setCaption("Empty Schema");
		}
	}

	MonitorSchemaWidget* schemaWidget = new MonitorSchemaWidget(tabSchema,
																m_schemaManager,
																m_appSignalController,
																m_tuningController,
																this);
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

void MonitorCentralWidget::slot_schemaList()
{
	// Activate tab it schema list already opened
	//
	for (int i = 0; i < this->count(); i++)
	{
		SchemaListWidget* w = dynamic_cast<SchemaListWidget*>(this->widget(i));
		if (w != nullptr)
		{
			setCurrentIndex(i);
			return;
		}
	}

	// Schemal list is not opened yet, create it and add to tab control
	//
	SchemaListWidget* w = new SchemaListWidget{this};
	w->setDetails(m_schemaManager->monitorConfigController()->schemasDetailsSet());

	connect(w, &SchemaListWidget::openSchemaRequest, this, &MonitorCentralWidget::slot_newSchemaTab);

	connect(m_schemaManager->monitorConfigController(), &MonitorConfigController::configurationUpdate, w,
			[this, w]()
			{
				w->setDetails(m_schemaManager->monitorConfigController()->schemasDetailsSet());
			});

	int index = this->addTab(w, tr("Schemas"));

	if (count() > 1 && tabsClosable() == false)
	{
		setTabsClosable(true);
		setMovable(true);
	}

	setCurrentIndex(index);

	emit signal_actionCloseTabUpdated(count() > 1);

	return;
}

void MonitorCentralWidget::slot_newSchemaTab(QString schemaId)
{
	int tabIndex = addSchemaTabPage(schemaId, {});

	// Switch to the new tab
	//
	if (tabIndex != -1)
	{
		setCurrentIndex(tabIndex);

		MonitorSchemaWidget* newTab = currentTab();
		Q_ASSERT(newTab);

		newTab->clientSchemaView()->setZoom(0, false);

		emit signal_schemaChanged(newTab->schemaId());		// Different schema could be set, it can happen if schema does not exist
		newTab->emitHistoryChanged();
	}

	return;
}

void MonitorCentralWidget::slot_newTab()
{
	MonitorSchemaWidget* curTabWidget = dynamic_cast<MonitorSchemaWidget*>(currentWidget());

	if (curTabWidget == nullptr)
	{
		// If Current tab not schema widget,
		// then create new empty or start schema widget
		//
		QString schemaId = m_schemaManager->monitorConfigController()->configurationStartSchemaId();

		int tabIndex = addSchemaTabPage(schemaId, {});

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
	}
	else
	{
		// Duplicate tab
		//
		slot_newSameTab(curTabWidget);
	}

	// Set zoom for new tab
	//
	if (auto newTabWidget = dynamic_cast<MonitorSchemaWidget*>(currentWidget());
		newTabWidget != nullptr)
	{
		newTabWidget->clientSchemaView()->setZoom(0);
	}

	return;
}

void MonitorCentralWidget::slot_closeCurrentTab()
{
	QWidget* curTabWidget = currentWidget();
	if (curTabWidget == nullptr)
	{
		Q_ASSERT(curTabWidget);
		return;
	}

	slot_closeTab(curTabWidget);

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

void MonitorCentralWidget::slot_zoomToFit()
{
	MonitorSchemaWidget* curTabWidget = dynamic_cast<MonitorSchemaWidget*>(currentWidget());

	if (curTabWidget == nullptr)
	{
		Q_ASSERT(curTabWidget);
		return;
	}

	curTabWidget->zoomToFit();
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

	tab->setSchema(schemaId, QStringList{});

	tab->emitHistoryChanged();

	return;
}

void MonitorCentralWidget::slot_signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu)
{
	currentTab()->signalContextMenu(signalList, {}, customMenu);
}

void MonitorCentralWidget::slot_signalInfo(QString signalId)
{
	currentTab()->signalInfo(signalId);
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

	QWidget* tabWidget = widget(index);
	if (tabWidget == nullptr)
	{
		return;
	}

	removeTab(index);
	delete tabWidget;

	if (count() <= 1)
	{
		// Hide close button to prevent blink
		//
		QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, tabBar());
		for (int i = 0; i < count(); i++)
		{
			QWidget* closeButton = tabBar()->tabButton(i, closeSide);
			if (closeButton != nullptr)
			{
				closeButton->setVisible(false);
			}
		}

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
			// it can be schema list
			//
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

		tabPage->setSchema(schemaToLoad, QStringList{});

		tabPage->clientSchemaView()->deleteControlWidgets();		// deleteControlWidgets after loading new schema, as it will delete old widgets and later they will be created
		tabPage->clientSchemaView()->updateControlWidgets(false);

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

void MonitorCentralWidget::slot_closeTab(QWidget* tabWidget)
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

void MonitorCentralWidget::slot_tabPageChanged(int index)
{
	MonitorSchemaWidget* schemaWidgetTab = currentTab();

	emit signal_tabPageChanged(schemaWidgetTab != nullptr);	// This signal is to enable/disable QActions

	// Show/hide close button for inactive tab bar
	//
	QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, this->tabBar());

	for (int i = 0; i < this->count(); i++)
	{
		QWidget* w = this->tabBar()->tabButton(i, closeSide);

		if (w != nullptr)
		{
			w->setVisible(i == index);
		}
	}

	// --
	//
	if (schemaWidgetTab != nullptr)
	{
		emit signal_schemaChanged(schemaWidgetTab->schemaId());
		schemaWidgetTab->emitHistoryChanged();
	}

	return;
}



