#include "SimSchemaView.h"
#include "SimSchemaManager.h"
#include "SimIdeSimulator.h"
#include "../../Simulator/SimOverrideSignals.h"
#include "../lib/AppSignalManager.h"
#include "../VFrame30/PropertyNames.h"



// MonitorView
//
SimSchemaView::SimSchemaView(SimSchemaManager* schemaManager, QWidget* parent)
	: VFrame30::ClientSchemaView(schemaManager, nullptr/*History navigation is not supported (now)*/, parent),
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

VFrame30::DrawMode SimSchemaView::drawMode() const
{
	return VFrame30::DrawMode::Simulator;
}

void SimSchemaView::updateScriptGlobalVars(QJSEngine& engine)
{
	VFrame30::ClientSchemaView::updateScriptGlobalVars(engine);

//	// create global variable "app"
//	//
//	{
//		QJSValue jsApp = engine.newQObject(&theApp);
//		QQmlEngine::setObjectOwnership(&theApp, QQmlEngine::CppOwnership);

//		engine.globalObject().setProperty(VFrame30::PropertyNames::scriptGlobalVariableApp, jsApp);
//	}

	// create global variable "tuning"
	//
	{
		QJSValue jsTuning = engine.newQObject(tuningController());
		QQmlEngine::setObjectOwnership(tuningController(), QQmlEngine::CppOwnership);

		engine.globalObject().setProperty(VFrame30::PropertyNames::scriptGlobalVariableTuning, jsTuning);
	}

	// Create global variable "signals"
	//
	{
		Q_ASSERT(m_scriptAppSignalController);

		QJSValue jsSignals = engine.newQObject(m_scriptAppSignalController.get());
		QQmlEngine::setObjectOwnership(m_scriptAppSignalController.get(), QQmlEngine::CppOwnership);

		engine.globalObject().setProperty(VFrame30::PropertyNames::scriptGlobalVariableSignals, jsSignals);
	}

	return;
}

void SimSchemaView::overrideSignalsChanged(QStringList /*addedAppSignalIds*/)
{
	QStringList ids = m_simulator->overrideSignals().overrideSignalIds();

	setHighlightIds(ids);

	return;
}


