#include "SimSelectSchemaPage.h"

SimSelectSchemaPage::SimSelectSchemaPage(SimIdeSimulator* simulator, QWidget* parent) :
	SimBasePage(simulator, parent)
{
	assert(m_simulator);

	m_schemaListWidget = new SchemaListWidget{
						 {SchemaListTreeColumns::SchemaID, SchemaListTreeColumns::Caption, SchemaListTreeColumns::Tags, SchemaListTreeColumns::Modules},
						 true,
						 parent};

	QGridLayout* layout = new QGridLayout;
	layout->addWidget(m_schemaListWidget);
	setLayout(layout);

	connect(m_simulator, &SimIdeSimulator::projectUpdated, this, &SimSelectSchemaPage::updateSchemaList);
	connect(m_schemaListWidget, &SchemaListWidget::openSchemaRequest, this, &SimSelectSchemaPage::openSchemaTabPage);

	updateSchemaList();

	return;
}

void SimSelectSchemaPage::updateSchemaList()
{
	if (m_simulator->isLoaded() == true)
	{
		m_schemaListWidget->setDetails(m_simulator->schemaDetails());
	}
	else
	{
		m_schemaListWidget->setDetails({});
	}

	return;
}
