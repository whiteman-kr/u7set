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

	// On start create an empty MonitorSchema and add a tab with this schema
	//
	addSchemaTabPage("EMPTYSCHEMA");
	addSchemaTabPage("EMPTYSCHEMA2");
	addSchemaTabPage("EMPTYSCHEMA3");

	// --
	//
	connect(this->tabBar(), &QTabBar::tabCloseRequested, this, &MonitorCentralWidget::slot_tabCloseRequested);

	connect(m_schemaManager, &SchemaManager::resetSchema, this, &MonitorCentralWidget::slot_resetSchema);
}

MonitorCentralWidget::~MonitorCentralWidget()
{
	qDebug() << Q_FUNC_INFO;
}

void MonitorCentralWidget::addSchemaTabPage(QString schemaId)
{
	std::shared_ptr<VFrame30::Schema> tabSchema = m_schemaManager->schema(schemaId);

	if (tabSchema == nullptr)
	{
		tabSchema = std::make_shared<VFrame30::MonitorSchema>();
		tabSchema->setSchemaID("EMPTYSCHEMA");
		tabSchema->setCaption("Empty Schema");
	}

	MonitorSchemaWidget* schemaWidget = new MonitorSchemaWidget(tabSchema, m_schemaManager);
	addTab(schemaWidget, tabSchema->caption());

	if (count() > 1 && tabsClosable() == false)
	{
		setTabsClosable(true);
		setMovable(true);
	}

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

	QWidget* tabWidget = widget(index);
	removeTab(index);
	delete tabWidget;

	if (count() <= 1)
	{
		setTabsClosable(false);
		setMovable(false);
	}

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

		std::shared_ptr<VFrame30::Schema> newSchema = m_schemaManager->schema(tabPage->schemaId());

		if (newSchema == nullptr)
		{
			// Load startSchemaId
			//
			newSchema = m_schemaManager->schema(startSchemaId);

			if (newSchema == nullptr)
			{
				// and there is no startSchemaId (((
				// Just create an empty schema
				//
				newSchema = std::make_shared<VFrame30::MonitorSchema>();
				newSchema->setSchemaID("EMPTYSCHEMA");
				newSchema->setCaption("Empty Schema");
			}
		}

		setTabText(i, newSchema->caption());
		tabPage->setSchema(newSchema);
	}

	return;
}


