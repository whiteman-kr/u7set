#include "Settings.h"
#include "MainWindow.h"

#include "SchemasWorkspace.h"

#include <QTreeWidget>

SchemasWorkspace::SchemasWorkspace(ConfigController* configController,
								   TuningSignalManager* tuningSignalManager,
								   TuningClientTcpClient* tuningTcpClient,
								   QWidget* parent) :
	QWidget(parent),
	m_tuningController(tuningSignalManager, tuningTcpClient),
	m_tuningSignalManager(tuningSignalManager),
	m_tuninTcpClient(tuningTcpClient),
	m_schemaManager(configController)
{
	if (configController == nullptr || m_tuningSignalManager== nullptr || m_tuninTcpClient == nullptr )
	{
		assert(configController);
		assert(m_tuningSignalManager);
		assert(m_tuninTcpClient);
		return;
	}

	if (theConfigSettings.schemas.empty() == true)
	{
		return;
	}

	// Create schema widgets and navigation controls

	QHBoxLayout* mainLayout = new QHBoxLayout(this);

	if (theConfigSettings.showSchemasList == true)
	{
		m_schemasList = new QTreeWidget();
		m_schemasList->setObjectName("SchemasTreeWidget");
		m_schemasList->setRootIsDecorated(false);

		QStringList headerLabels;
		headerLabels << tr("ID");
		headerLabels << tr("Caption");

		m_schemasList->setColumnCount(headerLabels.size());
		m_schemasList->setHeaderLabels(headerLabels);
		m_schemasList->setSelectionMode(QAbstractItemView::SingleSelection);

		for (const SchemaSettings& schemaID : theConfigSettings.schemas)
		{
			if (schemaID.m_id.isEmpty() == true)
			{
				assert(false);
				continue;
			}

			QStringList l;
			l << schemaID.m_id;
			l << schemaID.m_caption;

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

			if (item->text(0) == theConfigSettings.startSchemaID)
			{
				startSchemaItem = item;
				break;
			}
		}

		if (startSchemaItem == nullptr)
		{
			// No startSchemaID was found, show first
			if (m_schemasList->topLevelItemCount() == 0)
			{
				Q_ASSERT(false);
				return;
			}

			startSchemaItem = m_schemasList->topLevelItem(0);
		}

		if (startSchemaItem == nullptr)
		{
			Q_ASSERT(startSchemaItem);
			return;
		}

		startSchemaItem->setSelected(true);

		QString startSchemaID = startSchemaItem->text(0);
		if (startSchemaID.isEmpty() == true)
		{
			Q_ASSERT(false);
			return;
		}

		connect(m_schemasList, &QTreeWidget::itemSelectionChanged, this, &SchemasWorkspace::slot_itemSelectionChanged);

		//

		std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(startSchemaID);
		if (schema == nullptr)
		{
			assert(schema);
			return;
		}

		m_schemaWidget = new TuningSchemaWidget(m_tuningSignalManager, &m_tuningController, schema, &m_schemaManager, this);
		connect(m_schemaWidget, &TuningSchemaWidget::signal_schemaChanged, this, &SchemasWorkspace::slot_schemaChanged);

		m_hSplitter = new QSplitter(this);
		m_hSplitter->addWidget(m_schemasList);
		m_hSplitter->addWidget(m_schemaWidget);
		mainLayout->addWidget(m_hSplitter);

		m_hSplitter->restoreState(theSettings.m_schemasWorkspaceSplitterState);
	}
	else
	{
		if (theConfigSettings.showSchemasTabs == true)
		{
			// Create widgets sorted by id map

			std::map<QString, TuningSchemaWidget*> widgets;

			for (const SchemaSettings& schemaID : theConfigSettings.schemas)
			{
				if (schemaID.m_id.isEmpty() == true)
				{
					assert(false);
					continue;
				}

				std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(schemaID.m_id);

				TuningSchemaWidget* schemaWidget = new TuningSchemaWidget(m_tuningSignalManager, &m_tuningController, schema, &m_schemaManager, this);

				connect(schemaWidget, &TuningSchemaWidget::signal_schemaChanged, this, &SchemasWorkspace::slot_schemaChanged);

				widgets[schemaID.m_id] = schemaWidget;
			}

			// Add widgets to tab

			m_tabWidget = new QTabWidget();

			for (auto w : widgets)
			{
				TuningSchemaWidget* schemaWidget = w.second;

				m_tabWidget->addTab(schemaWidget, schemaWidget->caption());

				if (w.first == theConfigSettings.startSchemaID)
				{
					// Set current tab to startSchemaID
					//
					m_tabWidget->setCurrentIndex(m_tabWidget->count() - 1);
				}
			}

			mainLayout->addWidget(m_tabWidget);
		}
		else
		{
			// No tab

			SchemaSettings schemaID;

			for (int i = 0; i < theConfigSettings.schemas.size(); i++)
			{
				if (theConfigSettings.schemas[i].m_id == theConfigSettings.startSchemaID)
				{
					schemaID = theConfigSettings.schemas[i];
				}
			}

			if (schemaID.m_id.isEmpty() == true)
			{
				// No startSchemaID was found, show first

				schemaID = theConfigSettings.schemas[0];

				if (schemaID.m_id.isEmpty() == true)
				{
					assert(false);
					return;
				}
			}

			std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(schemaID.m_id);

			m_schemaWidget = new TuningSchemaWidget(m_tuningSignalManager, &m_tuningController, schema, &m_schemaManager, this);

			mainLayout->addWidget(m_schemaWidget);
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

	if (theConfigSettings.showSchemasList == true)
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
		if (theConfigSettings.showSchemasTabs == true)
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
