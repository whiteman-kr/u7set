#include "Stable.h"
#include "Settings.h"
#include "MainWindow.h"

#include "SchemasWorkspace.h"

SchemasWorkspace::SchemasWorkspace(ConfigController* configController, TuningObjectManager *tuningObjectManager, const TuningObjectStorage *objects, QWidget *parent):
	m_tuningObjectManager(tuningObjectManager),
	m_objects(*objects),
	QWidget(parent)
{
	m_schemaStorage = new SchemaStorage(configController);

	assert(m_tuningObjectManager);
	assert(objects);

	if (theConfigSettings.showSchemasList == true)
	{
		m_schemasList = new QTreeWidget();

		m_schemasList->setRootIsDecorated(false);

		m_schemasList->setHeaderHidden(true);

		connect(m_schemasList, &QTreeWidget::currentItemChanged, this, &SchemasWorkspace::slot_schemaListSelectionChanged);

		QString firstSchemaID;

		for (auto schemaID : theConfigSettings.schemasID)
		{
			m_schemasList->addTopLevelItem(new QTreeWidgetItem(QStringList()<<schemaID));

			if (firstSchemaID.isEmpty() == true)
			{
				firstSchemaID = schemaID;
			}
		}

		std::shared_ptr<VFrame30::Schema> schema = m_schemaStorage->schema(firstSchemaID);

		m_schemaWidget = new TuningSchemaWidget(m_tuningObjectManager, schema, m_schemaStorage);

		m_hSplitter = new QSplitter(this);

		m_hSplitter->addWidget(m_schemasList);

		m_hSplitter->addWidget(m_schemaWidget);

		QHBoxLayout* layout = new QHBoxLayout(this);

		layout->addWidget(m_hSplitter);
	}
	else
	{

		QTabWidget* tab = new QTabWidget();

		for (auto schemaID : theConfigSettings.schemasID)
		{
			std::shared_ptr<VFrame30::Schema> schema = m_schemaStorage->schema(schemaID);

			TuningSchemaWidget* schemaWidget = new TuningSchemaWidget(m_tuningObjectManager,schema, m_schemaStorage);

			tab->addTab(schemaWidget, schemaID);
		}
		QHBoxLayout* layout = new QHBoxLayout(this);

		layout->addWidget(tab);
	}

}

SchemasWorkspace::~SchemasWorkspace()
{
	delete m_schemaStorage;
}

void SchemasWorkspace::slot_schemaListSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem* /*previous*/)
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

	m_schemaWidget->tuningSchemaView()->setSchema(schemaId);

}
