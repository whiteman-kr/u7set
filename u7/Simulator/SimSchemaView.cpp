#include "SimSchemaView.h"
#include "SimSchemaManager.h"
#include "SimIdeSimulator.h"
#include "../../Simulator/SimOverrideSignals.h"
#include "../lib/AppSignalManager.h"
#include "../VFrame30/PropertyNames.h"



// MonitorView
//
SimSchemaView::SimSchemaView(SimSchemaManager* schemaManager, QWidget* parent)
	: VFrame30::ClientSchemaView(schemaManager, parent),
	  m_simulator(schemaManager->simulator())
{
	Q_ASSERT(schemaManager);
	Q_ASSERT(m_simulator);

	connect(&m_simulator->overrideSignals(), &Sim::OverrideSignals::signalsChanged, this, &SimSchemaView::overrideSignalsChanged);

	return;
}

SimSchemaView::~SimSchemaView()
{
	return;
}

void SimSchemaView::overrideSignalsChanged(QStringList /*addedAppSignalIds*/)
{
	QStringList ids = m_simulator->overrideSignals().overrideSignalIds();

	setHighlightIds(ids);

	return;
}


