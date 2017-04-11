#include "Stable.h"
#include "Settings.h"
#include "MainWindow.h"

#include "SchemasWorkspace.h"

SchemasWorkspace::SchemasWorkspace(const TuningObjectStorage *objects, SchemaStorage *schemaStorage, QWidget *parent):
	m_schemaStorage(schemaStorage),
	m_objects(*objects),
	QWidget(parent)
{
	assert(schemaStorage);
	assert(objects);

	if (theConfigSettings.showSchemasList == true)
	{
		m_schemasList = new QTreeWidget();

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

		std::shared_ptr<VFrame30::Schema> schema = schemaStorage->schema(firstSchemaID);

		m_schemaWidget = new TuningSchemaWidget(schema, schemaStorage);

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
			std::shared_ptr<VFrame30::Schema> schema = schemaStorage->schema(schemaID);

			TuningSchemaWidget* schemaWidget = new TuningSchemaWidget(schema, schemaStorage);

			tab->addTab(schemaWidget, schemaID);
		}
		QHBoxLayout* layout = new QHBoxLayout(this);

		layout->addWidget(tab);
	}

}

SchemasWorkspace::~SchemasWorkspace()
{

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

	m_schemaWidget->setSchema(m_schemaStorage->schema(schemaId), true);

}
