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

	QHBoxLayout* mainLayout = new QHBoxLayout(this);

	if (theConfigSettings.showSchemasList == true)
	{
		m_schemasList = new QTreeWidget();
		m_schemasList->setRootIsDecorated(false);

		QStringList headerLabels;
		headerLabels << tr("ID");
		headerLabels << tr("Caption");

		m_schemasList->setColumnCount(headerLabels.size());
		m_schemasList->setHeaderLabels(headerLabels);
		m_schemasList->setSelectionMode(QAbstractItemView::SingleSelection);
		m_schemasList->setSortingEnabled(true);
		m_schemasList->sortByColumn(0, Qt::AscendingOrder);

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

		m_schemasList->resizeColumnToContents(0);
		m_schemasList->resizeColumnToContents(1);

		if (m_schemasList->topLevelItemCount() == 0)
		{
			return;
		}

		QTreeWidgetItem* firstItem = m_schemasList->topLevelItem(0);
		if (firstItem == nullptr)
		{
			Q_ASSERT(firstItem);
			return;
		}

		firstItem->setSelected(true);

		QString firstSchemaID = firstItem->text(0);
		if (firstSchemaID.isEmpty() == true)
		{
			return;
		}

		connect(m_schemasList, &QTreeWidget::itemSelectionChanged, this, &SchemasWorkspace::slot_itemSelectionChanged);

		//

		std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(firstSchemaID);
		if (schema == nullptr)
		{
			assert(schema);
			return;
		}

		m_schemaWidget = new TuningSchemaWidget(m_tuningSignalManager, &m_tuningController, schema, &m_schemaManager);
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
				std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(schemaID.m_id);

				TuningSchemaWidget* schemaWidget = new TuningSchemaWidget(m_tuningSignalManager, &m_tuningController, schema, &m_schemaManager);

				connect(schemaWidget, &TuningSchemaWidget::signal_schemaChanged, this, &SchemasWorkspace::slot_schemaChanged);

				widgets[schemaID.m_id] = schemaWidget;
			}

			// Add widgets to tab

			m_tabWidget = new QTabWidget();

			for (auto w : widgets)
			{
				TuningSchemaWidget* schemaWidget = w.second;

				m_tabWidget->addTab(schemaWidget, schemaWidget->caption());
			}

			mainLayout->addWidget(m_tabWidget);
		}
		else
		{
			// No tab

			const SchemaSettings& schemaID = theConfigSettings.schemas[0];

			std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(schemaID.m_id);

			m_schemaWidget = new TuningSchemaWidget(m_tuningSignalManager, &m_tuningController, schema, &m_schemaManager);

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
	m_schemaWidget->setSchema(schemaId);

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
