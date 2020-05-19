#include "SimSchemaPage.h"


SimSchemaPage::SimSchemaPage(std::shared_ptr<VFrame30::Schema> schema,
							 SimIdeSimulator* simulator,
							 SimSchemaManager* schemaManager,
							 VFrame30::AppSignalController* appSignalController,
							 VFrame30::TuningController* tuningController,
							 QWidget* parent)
	: SimBasePage(simulator, parent)
{
	assert(schema);
	assert(schemaManager);
	assert(appSignalController);
	assert(tuningController);
	assert(m_simulator);

	// --
	//
	m_schemaWidget = new SimSchemaWidget(schema, schemaManager, appSignalController, tuningController, m_simulator);

	QGridLayout* layout = new QGridLayout();
	layout->addWidget(m_schemaWidget, 0, 0, 1, 1);

	layout->setContentsMargins(2, 0, 2, 2);

	setLayout(layout);

	// --
	//
	connect(&simulator->control(), &Sim::Control::stateChanged, this, &SimSchemaPage::controlStateChanged);

	SimSchemaPage::controlStateChanged(simulator->control().state());	// Slots catches only changes of state, so init the firts time

	return;
}

void SimSchemaPage::controlStateChanged(Sim::SimControlState state)
{
	m_schemaWidget->clientSchemaView()->setPeriodicUpdate(state == Sim::SimControlState::Run);
	m_schemaWidget->clientSchemaView()->update();	// make an update, just in case

	return;
}

QString SimSchemaPage::schemaId() const
{
	return m_schemaWidget->schemaId();
}

SimSchemaWidget* SimSchemaPage::simSchemaWidget()
{
	return m_schemaWidget;
}

const SimSchemaWidget* SimSchemaPage::simSchemaWidget() const
{
	return m_schemaWidget;
}
