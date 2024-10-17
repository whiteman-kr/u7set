#include "SimSchemaPage.h"
#include "SimSchemaView.h"
#include <SimulatorUi/SimIdeSimulator.h>
#include <Simulator/SimControl.h>

#include <QGridLayout>

namespace SimUi
{
	SimSchemaPage::SimSchemaPage(std::shared_ptr<VFrame30::Schema> schema,
								 SimIdeSimulator* simulator,
								 SimSchemaManager* schemaManager,
								 VFrame30::AppSignalController* appSignalController,
								 QWidget* parent) :
		SimBasePage(simulator, parent)
	{
		assert(schema);
		assert(schemaManager);
		assert(appSignalController);
		assert(m_simulator);

		// --
		//
		m_schemaWidget = new SimSchemaWidget{schema, schemaManager, appSignalController, m_simulator, this};

		QGridLayout* layout = new QGridLayout();
		layout->addWidget(m_schemaWidget, 0, 0, 1, 1);

		setLayout(layout);

		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);

		// --
		//
		connect(&simulator->control(), &Sim::Control::stateChanged, this, &SimSchemaPage::controlStateChanged);

		SimSchemaPage::controlStateChanged(simulator->control().state()); // Slots catches only changes of state, so init the first time

		return;
	}

	void SimSchemaPage::controlStateChanged(Sim::SimControlState state)
	{
		m_schemaWidget->clientSchemaView()->setPeriodicUpdate(state == Sim::SimControlState::Run);
		m_schemaWidget->clientSchemaView()->update(); // make an update, just in case

		return;
	}

	QString SimSchemaPage::schemaId() const
	{
		return m_schemaWidget->schemaId();
	}

	const QStringList& SimSchemaPage::highlightIds() const
	{
		return m_schemaWidget->simSchemaView()->highlightIds();
	}

	void SimSchemaPage::setHighlightIds(const QStringList& value)
	{
		return m_schemaWidget->simSchemaView()->setHighlightIds(value);
	}

	SimSchemaWidget* SimSchemaPage::simSchemaWidget()
	{
		return m_schemaWidget;
	}

	const SimSchemaWidget* SimSchemaPage::simSchemaWidget() const
	{
		return m_schemaWidget;
	}
} // namespace SimUi