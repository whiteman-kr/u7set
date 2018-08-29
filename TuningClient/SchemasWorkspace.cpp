#include "Settings.h"
#include "MainWindow.h"

#include "SchemasWorkspace.h"

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
	assert(m_tuningSignalManager);
	assert(m_tuninTcpClient);

	if (theConfigSettings.showSchemasList == true)
	{
		m_schemasList = new QTreeWidget();
		m_schemasList->setColumnCount(2);

		m_schemasList->setRootIsDecorated(false);

		m_schemasList->setHeaderHidden(true);

		connect(m_schemasList, &QTreeWidget::currentItemChanged, this, &SchemasWorkspace::slot_schemaListSelectionChanged);

		QString firstSchemaID;

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

			if (firstSchemaID.isEmpty() == true)
			{
				firstSchemaID = schemaID.m_id;
			}
		}

		m_schemasList->resizeColumnToContents(0);
		m_schemasList->resizeColumnToContents(1);

		if (firstSchemaID == "")
		{
			assert(false);
			return;
		}

		std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(firstSchemaID);

		if (schema == nullptr)
		{
			assert(schema);
			return;
		}

		m_schemaWidget = new TuningSchemaWidget(m_tuningSignalManager, &m_tuningController, schema, &m_schemaManager);

		m_hSplitter = new QSplitter(this);

		m_hSplitter->addWidget(m_schemasList);

		m_hSplitter->addWidget(m_schemaWidget);

		QHBoxLayout* layout = new QHBoxLayout(this);

		layout->addWidget(m_hSplitter);

		m_hSplitter->restoreState(theSettings.m_schemasWorkspaceSplitterState);
	}
	else
	{

		QTabWidget* tab = new QTabWidget();

		for (const SchemaSettings& schemaID : theConfigSettings.schemas)
		{
			std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(schemaID.m_id);

			TuningSchemaWidget* schemaWidget = new TuningSchemaWidget(m_tuningSignalManager, &m_tuningController, schema, &m_schemaManager);

			tab->addTab(schemaWidget, schemaID.m_caption);
		}
		QHBoxLayout* layout = new QHBoxLayout(this);

		layout->addWidget(tab);
	}

}

SchemasWorkspace::~SchemasWorkspace()
{
	if (m_hSplitter != nullptr)
	{
		theSettings.m_schemasWorkspaceSplitterState = m_hSplitter->saveState();
	}
}

void SchemasWorkspace::slot_schemaListSelectionChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
{
	if (current == nullptr)
	{
		return;
	}

	if (m_schemaWidget == nullptr)
	{
		assert(m_schemaWidget);
		return;
	}

	QString schemaId = current->text(0);

	m_schemaWidget->setSchema(schemaId);

}
