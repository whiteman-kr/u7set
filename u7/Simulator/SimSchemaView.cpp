#include "SimSchemaView.h"
#include "SimSchemaManager.h"
#include "SimIdeSimulator.h"
#include "ScriptSimApplication.h"
#include "../../Simulator/SimOverrideSignals.h"
#include "../../Simulator/SimSoftware.h"
#include "../VFrame30/PropertyNames.h"



// MonitorView
//
SimSchemaView::SimSchemaView(SimSchemaManager* schemaManager, QWidget* parent)
	: VFrame30::ClientSchemaView(schemaManager, nullptr/*History navigation is not supported (now)*/, nullptr, parent),
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

QString SimSchemaView::monitorId() const
{
	return m_monitorId;
}

void SimSchemaView::setMonitorId(QString equipmentId, bool emitUpdate)
{
	if (m_simulator == nullptr)
	{
		assert(m_simulator);
		return;
	}

	m_monitorId = equipmentId;

	if (auto monitor = m_simulator->software().monitor(m_monitorId);
		monitor != nullptr)
	{
		setGlobalScript(monitor->globalScript());
	}

	if (emitUpdate == true)
	{
		// It will update all schemas, evaluate new GlobalScript and execute onConfigurationArrived
		//
		m_simulator->projectUpdated();
	}

	return;
}

void SimSchemaView::updateScriptGlobalVars(QJSEngine& engine)
{
	VFrame30::ClientSchemaView::updateScriptGlobalVars(engine);

	// create global variable "app"
	//
	{
		ScriptSimApplication* scriptApp = new ScriptSimApplication(this);
		QJSValue jsApp = engine.newQObject(scriptApp);

		engine.globalObject().setProperty(VFrame30::PropertyNames::scriptGlobalVariableApp, jsApp);
	}

	// create global variable "tuning"
	//
	{
		QJSValue jsTuning = engine.newQObject(tuningController());
		QJSEngine::setObjectOwnership(tuningController(), QJSEngine::CppOwnership);

		engine.globalObject().setProperty(VFrame30::PropertyNames::scriptGlobalVariableTuning, jsTuning);
	}

	// Create global variable "signals"
	//
	{
		Q_ASSERT(m_scriptAppSignalController);

		QJSValue jsSignals = engine.newQObject(m_scriptAppSignalController.get());
		QJSEngine::setObjectOwnership(m_scriptAppSignalController.get(), QJSEngine::CppOwnership);

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


