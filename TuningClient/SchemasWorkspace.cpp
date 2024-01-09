#include "Settings.h"

#include "SchemasWorkspace.h"

#include <QTreeWidget>

SchemasWorkspace::SchemasWorkspace(TuningConfigController& configController,
								   const QString& caption,
								   const QStringList& schemasTags,
								   QString startSchemaId,
								   ILogFile* logFile,
								   QWidget* parent) :
	QWidget(parent),
	m_configController(configController),
	m_logController(logFile),
	m_schemaManager(configController),
	m_caption(caption),
	m_startSchemaId(startSchemaId),
	m_schemasTags(schemasTags)
{
	if (configController.schemaCount() == 0)
	{
		return;
	}

	// Create schema widgets and navigation controls

	const TuningClientSettings& clientSettings = m_configController.configuration().clientSettings;

	if (clientSettings.showSchemasList == true)
	{
		createSchemasList();
	}
	else
	{
		if (clientSettings.showSchemasTabs == true)
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
		TuningClientAppSettings::instance().user().m_schemasWorkspaceSplitterState = m_hSplitter->saveState();
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

	m_schemaWidget->setSchema(schemaId, QStringList{}, false);

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

	const TuningClientSettings& clientSettings = m_configController.configuration().clientSettings;

	QString id = schema->schemaId();
	QString caption = schema->caption();

	if (clientSettings.showSchemasList == true)
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
		if (clientSettings.showSchemasTabs == true)
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

	m_schemasList->setColumnCount(static_cast<int>(headerLabels.size()));
	m_schemasList->setHeaderLabels(headerLabels);
	m_schemasList->setSelectionMode(QAbstractItemView::SingleSelection);

	for (int i = 0; i < m_configController.schemaCount(); i++)
	{
		QString schemaId = m_configController.schemaIdByIndex(i);

		if (m_schemasTags.empty() == false)
		{
			if (m_configController.schemaHasTags(schemaId, m_schemasTags) == false)
			{
				continue;
			}
		}

		QStringList l;
		l << schemaId;
		l << m_configController.schemaCaptionByIndex(i);

		QTreeWidgetItem* item = new QTreeWidgetItem(l);
		m_schemasList->addTopLevelItem(item);
	}

	m_schemasList->setSortingEnabled(true);
	m_schemasList->sortByColumn(0, Qt::AscendingOrder);
	m_schemasList->resizeColumnToContents(0);
	m_schemasList->resizeColumnToContents(1);

	// Show start schema or first schema in the list

	// Create a dummy context, later it will be changed to the normal one inside TuningSchemaWidget
	// constructor, as TuningSchemaView is cretaed inside TuningSchemaWidget().
	//
	auto dummyContext = VFrame30::Context::create(nullptr, nullptr, nullptr, nullptr);

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

		std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(startSchemaID, dummyContext);
		if (schema == nullptr)
		{
			assert(schema);
			return;
		}

		m_schemaWidget = new TuningSchemaWidget(m_configController,
												&m_logController,
												schema,
												m_schemaManager,
												this);

		m_hSplitter->addWidget(m_schemaWidget);

		connect(m_schemaWidget, &TuningSchemaWidget::signal_schemaChanged, this, &SchemasWorkspace::slot_schemaChanged);
		connect(m_schemasList, &QTreeWidget::itemSelectionChanged, this, &SchemasWorkspace::slot_itemSelectionChanged);
	}

	mainLayout->addWidget(m_hSplitter);

	m_hSplitter->restoreState(TuningClientAppSettings::instance().user().m_schemasWorkspaceSplitterState);
}

void SchemasWorkspace::createSchemasTabs()
{
	QHBoxLayout* mainLayout = new QHBoxLayout(this);

	// Create widgets sorted by id map

	// Create a dummy context, later it will be changed to the normal one inside TuningSchemaWidget
	// constructor, as TuningSchemaView is created inside TuningSchemaWidget().
	//
	auto dummyContext = VFrame30::Context::create(nullptr, nullptr, nullptr, nullptr);

	std::map<QString, TuningSchemaWidget*> widgets;

	for (int i = 0; i < m_configController.schemaCount(); i++)
	{
		QString schemaId = m_configController.schemaIdByIndex(i);

		if (m_schemasTags.empty() == false && m_configController.schemaHasTags(schemaId, m_schemasTags) == false)
		{
			continue;
		}

		std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(schemaId, dummyContext);

		TuningSchemaWidget* schemaWidget = new TuningSchemaWidget(m_configController,
																  &m_logController,
																  schema,
																  m_schemaManager, this);

		connect(schemaWidget, &TuningSchemaWidget::signal_schemaChanged, this, &SchemasWorkspace::slot_schemaChanged);

		widgets[schemaId] = schemaWidget;
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

	QString startSchemaID;

	for (int i = 0; i < m_configController.schemaCount(); i++)
	{
		QString schemaId = m_configController.schemaIdByIndex(i);

		if (m_schemasTags.empty() == false && m_configController.schemaHasTags(schemaId, m_schemasTags) == false)
		{
			continue;
		}

		if (schemaId == m_startSchemaId)
		{
			startSchemaID = schemaId;
		}
	}

	if (startSchemaID.isEmpty() == true)
	{
		// No startSchemaID was found, show first

		for (int i = 0; i < m_configController.schemaCount(); i++)
		{
			startSchemaID = m_configController.schemaIdByIndex(i);

			if (m_schemasTags.empty() == false && m_configController.schemaHasTags(startSchemaID, m_schemasTags) == false)
			{
				continue;
			}

			break;
		}
	}

	if (startSchemaID.isEmpty() == true)
	{
		// No schema to view

		QLabel* emptyLabel = new QLabel(tr("No schemas exist for current page"));
		emptyLabel->setAlignment(Qt::AlignCenter);
		mainLayout->addWidget(emptyLabel);
		return;
	}

	// Create a dummy context, later it will be changed to the normal one inside TuningSchemaWidget
	// constructor, as TuningSchemaView is cretaed inside TuningSchemaWidget().
	//
	auto dummyContext = VFrame30::Context::create(nullptr, nullptr, nullptr, nullptr);
	std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(startSchemaID, dummyContext);

	m_schemaWidget = new TuningSchemaWidget(m_configController,
											&m_logController,
											schema,
											m_schemaManager,
											this);

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
