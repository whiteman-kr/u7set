#include "SimulatorSchemaPage.h"
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QMenu>

SimulatorSchemaPage::SimulatorSchemaPage(std::shared_ptr<VFrame30::Schema> schema,
										 std::shared_ptr<SimIdeSimulator> simulator,
										 SimSchemaManager* schemaManager,
										 VFrame30::AppSignalController* appSignalController,
										 VFrame30::TuningController* tuningController,
										 QWidget* parent)
	: SimulatorBasePage(simulator, parent)
{
	assert(schema);
	assert(schemaManager);
	assert(appSignalController);
	assert(tuningController);
	assert(m_simulator);

	// --
	//
	m_schemaWidget = new SimSchemaWidget(schema, schemaManager, appSignalController, tuningController);

	QGridLayout* layout = new QGridLayout();
	layout->addWidget(m_schemaWidget, 0, 0, 1, 1);

	layout->setContentsMargins(2, 0, 2, 2);

	setLayout(layout);

	return;
}

QString SimulatorSchemaPage::schemaId() const
{
	return m_schemaWidget->schemaId();
}
