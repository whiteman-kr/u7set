#include "Settings.h"

#include "SchemasWorkspace.h"

#include <QTreeWidget>

SchemasWorkspace::SchemasWorkspace(ConfigController* configController,
								   TuningSignalManager* tuningSignalManager,
								   TuningClientTcpClient* tuningTcpClient,
								   const QString& caption,
								   const QStringList& schemasTags,
								   QString startSchemaId,
								   ILogFile* logFile,
								   QWidget* parent) :
	QWidget(parent),
    m_tuningController(tuningSignalManager, tuningTcpClient),
	m_logController(logFile),
	m_tuningSignalManager(tuningSignalManager),
	m_tuningTcpClient(tuningTcpClient),
	m_schemaManager(configController),
	m_caption(caption),
	m_startSchemaId(startSchemaId),
	m_schemasTags(schemasTags)
{
	if (configController == nullptr || m_tuningSignalManager== nullptr || m_tuningTcpClient == nullptr )
	{
		assert(configController);
		assert(m_tuningSignalManager);
		assert(m_tuningTcpClient);
		return;
	}

	if (theConfigSettings.schemas.empty() == true)
	{
		return;
	}

	// Create schema widgets and navigation controls

	if (theConfigSettings.clientSettings.showSchemasList == true)
	{
		createSchemasList();
	}
	else
	{
		if (theConfigSettings.clientSettings.showSchemasTabs == true)
		{
			createSchemasTabs();
		}
		else
		{
			createSchemasView();
		}
	}

	return;
}

SchemasWorkspace::~SchemasWorkspace()
{
	if (m_hSplitter != nullptr)
	{
		theSettings.m_schemasWorkspaceSplitterState = m_hSplitter->saveState();
	}
}

const QString& SchemasWorkspace::caption() const
{
	return m_caption;
}

void SchemasWorkspace::slot_itemSelectionChanged()
{
	if (m_schemaWidget == nullptr)
	{
		assert(m_schemaWidget);
		return;
	}

	if (m_schemasList->selectedItems().isEmpty() == true)
	{
		return;
	}

	QTreeWidgetItem* selectedItem = m_schemasList->selectedItems().at(0);
	if (selectedItem == nullptr)
	{
		Q_ASSERT(selectedItem);
		return;
	}

	QString schemaId = selectedItem->text(0);

	double zoom = m_schemaWidget->zoom();

	m_schemaWidget->setSchema(schemaId, QStringList{});

	m_schemaWidget->setZoom(zoom, true);

}


void SchemasWorkspace::slot_schemaChanged(VFrame30::ClientSchemaWidget* widget, VFrame30::Schema* schema)
{
	if (widget == nullptr || schema == nullptr)
	{
		Q_ASSERT(widget);
		Q_ASSERT(schema);
		return;
	}

	QString id = schema->schemaId();
	QString caption = schema->caption();

	if (theConfigSettings.clientSettings.showSchemasList == true)
	{
		if (m_schemasList == nullptr)
		{
			Q_ASSERT(m_schemasList);
			return;
		}

		m_schemasList->blockSignals(true);
		m_schemasList->clearSelection();

		int count = m_schemasList->topLevelItemCount();
		for (int i = 0; i < count; i++)
		{
			QTreeWidgetItem* item = m_schemasList->topLevelItem(i);

			if (item->text(0) == id && item->text(1) == caption)
			{
				item->setSelected(true);
				break;
			}
		}

		m_schemasList->blockSignals(false);
	}
	else
	{
		if (theConfigSettings.clientSettings.showSchemasTabs == true)
		{
			if (m_tabWidget == nullptr)
			{
				Q_ASSERT(m_tabWidget);
				return;
			}

			QWidget* w = m_tabWidget->currentWidget();
			if (w == nullptr)
			{
				Q_ASSERT(w);
				return;
			}

			TuningSchemaWidget* schemaWidget = dynamic_cast<TuningSchemaWidget*>(w);
			if (schemaWidget == nullptr)
			{
				Q_ASSERT(schemaWidget);
				return;
			}

			m_tabWidget->setTabText(m_tabWidget->currentIndex(), caption);
		}
		else
		{
			// No tab, nothing to do
		}
	}

	return;
}

void SchemasWorkspace::zoomIn()
{
	TuningSchemaWidget* w = activeSchemaWidget();
	if (w == nullptr)
	{
		Q_ASSERT(w);
		return;
	}
	w->zoomIn();
}

void SchemasWorkspace::zoomOut()
{
	TuningSchemaWidget* w = activeSchemaWidget();
	if (w == nullptr)
	{
		Q_ASSERT(w);
		return;
	}
	w->zoomOut();
}

void SchemasWorkspace::zoom100()
{
	TuningSchemaWidget* w = activeSchemaWidget();
	if (w == nullptr)
	{
		Q_ASSERT(w);
		return;
	}
	w->zoom100();
}

void SchemasWorkspace::zoomToFit()
{
	TuningSchemaWidget* w = activeSchemaWidget();
	if (w == nullptr)
	{
		Q_ASSERT(w);
		return;
	}
	w->zoomToFit();
}

void SchemasWorkspace::createSchemasList()
{
	QHBoxLayout* mainLayout = new QHBoxLayout(this);

	m_hSplitter = new QSplitter(this);

	m_schemasList = new QTreeWidget();
	m_hSplitter->addWidget(m_schemasList);

	m_schemasList->setObjectName("SchemasTreeWidget");
	m_schemasList->setRootIsDecorated(false);

	QStringList headerLabels;
	headerLabels << tr("ID");
	headerLabels << tr("Caption");

	m_schemasList->setColumnCount(headerLabels.size());
	m_schemasList->setHeaderLabels(headerLabels);
	m_schemasList->setSelectionMode(QAbstractItemView::SingleSelection);

	for (const SchemaInfo& schemaInfo : theConfigSettings.schemas)
	{
		if (schemaInfo.m_id.isEmpty() == true)
		{
			assert(false);
			continue;
		}

		if (m_schemasTags.empty() == false && schemaInfo.hasAnyTag(m_schemasTags) == false)
		{
			continue;
		}

		QStringList l;
		l << schemaInfo.m_id;
		l << schemaInfo.m_caption;

		QTreeWidgetItem* item = new QTreeWidgetItem(l);
		m_schemasList->addTopLevelItem(item);
	}

	m_schemasList->setSortingEnabled(true);
	m_schemasList->sortByColumn(0, Qt::AscendingOrder);
	m_schemasList->resizeColumnToContents(0);
	m_schemasList->resizeColumnToContents(1);

	// Show start schema or first schema in the list

	QTreeWidgetItem* startSchemaItem = nullptr;

	int count = m_schemasList->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_schemasList->topLevelItem(i);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}

		if (item->text(0) == m_startSchemaId)
		{
			startSchemaItem = item;
			break;
		}
	}

	if (startSchemaItem == nullptr)
	{
		if (m_schemasList->topLevelItemCount() != 0)
		{
			startSchemaItem = m_schemasList->topLevelItem(0);
		}
	}

	if (startSchemaItem == nullptr)
	{
		QLabel* emptyLabel = new QLabel(tr("No schemas exist for current page"));
		emptyLabel->setAlignment(Qt::AlignCenter);
		m_hSplitter->addWidget(emptyLabel);
	}
	else
	{
		startSchemaItem->setSelected(true);

		QString startSchemaID = startSchemaItem->text(0);
		if (startSchemaID.isEmpty() == true)
		{
			Q_ASSERT(false);
			return;
		}

		//

		std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(startSchemaID);
		if (schema == nullptr)
		{
			assert(schema);
			return;
		}

		m_schemaWidget = new TuningSchemaWidget(m_tuningSignalManager, &m_tuningController, &m_logController, schema, &m_schemaManager, this);
		m_hSplitter->addWidget(m_schemaWidget);

		connect(m_schemaWidget, &TuningSchemaWidget::signal_schemaChanged, this, &SchemasWorkspace::slot_schemaChanged);
		connect(m_schemasList, &QTreeWidget::itemSelectionChanged, this, &SchemasWorkspace::slot_itemSelectionChanged);
	}

	mainLayout->addWidget(m_hSplitter);

	m_hSplitter->restoreState(theSettings.m_schemasWorkspaceSplitterState);
}

void SchemasWorkspace::createSchemasTabs()
{
	QHBoxLayout* mainLayout = new QHBoxLayout(this);

	// Create widgets sorted by id map

	std::map<QString, TuningSchemaWidget*> widgets;

	for (const SchemaInfo& schemaInfo : theConfigSettings.schemas)
	{
		if (schemaInfo.m_id.isEmpty() == true)
		{
			assert(false);
			continue;
		}

		if (m_schemasTags.empty() == false && schemaInfo.hasAnyTag(m_schemasTags) == false)
		{
			continue;
		}

		std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(schemaInfo.m_id);

		TuningSchemaWidget* schemaWidget = new TuningSchemaWidget(m_tuningSignalManager, &m_tuningController, &m_logController,schema, &m_schemaManager, this);

		connect(schemaWidget, &TuningSchemaWidget::signal_schemaChanged, this, &SchemasWorkspace::slot_schemaChanged);

		widgets[schemaInfo.m_id] = schemaWidget;
	}

	// Add widgets to tab

	if (widgets.empty() == true)
	{
		QLabel* emptyLabel = new QLabel(tr("No schemas exist for current page"));
		emptyLabel->setAlignment(Qt::AlignCenter);
		mainLayout->addWidget(emptyLabel);
	}
	else
	{
		m_tabWidget = new QTabWidget();

		for (auto w : widgets)
		{
			TuningSchemaWidget* schemaWidget = w.second;

			m_tabWidget->addTab(schemaWidget, schemaWidget->caption());

			if (w.first == m_startSchemaId)
			{
				// Set current tab to startSchemaID
				//
				m_tabWidget->setCurrentIndex(m_tabWidget->count() - 1);
			}
		}
		mainLayout->addWidget(m_tabWidget);
	}
}

void SchemasWorkspace::createSchemasView()
{
	QHBoxLayout* mainLayout = new QHBoxLayout(this);

	// No tab

	QString schemaID;

	for (const SchemaInfo& schemaInfo : theConfigSettings.schemas)
	{
		if (m_schemasTags.empty() == false && schemaInfo.hasAnyTag(m_schemasTags) == false)
		{
			continue;
		}

		if (schemaInfo.m_id == m_startSchemaId)
		{
			schemaID = schemaInfo.m_id;
		}
	}

	if (schemaID.isEmpty() == true)
	{
		// No startSchemaID was found, show first

		for (const SchemaInfo& schemaInfo : theConfigSettings.schemas)
		{
			if (m_schemasTags.empty() == false && schemaInfo.hasAnyTag(m_schemasTags) == false)
			{
				continue;
			}

			schemaID = schemaInfo.m_id;
			break;
		}
	}

	if (schemaID.isEmpty() == true)
	{
		// No schema to view

		QLabel* emptyLabel = new QLabel(tr("No schemas exist for current page"));
		emptyLabel->setAlignment(Qt::AlignCenter);
		mainLayout->addWidget(emptyLabel);

		return;
	}

	std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(schemaID);

	m_schemaWidget = new TuningSchemaWidget(m_tuningSignalManager, &m_tuningController, &m_logController, schema, &m_schemaManager, this);

	mainLayout->addWidget(m_schemaWidget);
}

TuningSchemaWidget* SchemasWorkspace::activeSchemaWidget()
{
	if (m_schemaWidget != nullptr)
	{
		return m_schemaWidget;
	}

	TuningSchemaWidget* w = dynamic_cast<TuningSchemaWidget*>(m_tabWidget->currentWidget());
	if (w == nullptr)
	{
		Q_ASSERT(w);
		return nullptr;
	}
	return w;
}
