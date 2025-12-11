#include "SimSchemaView.h"
#include "ScriptSimApplication.h"
#include "SimSchemaManager.h"

#include <SimulatorUi/SimIdeSimulator.h>

#include <HardwareLib/ScriptEquipment.h>
#include <SimulatorLib/SimOverrideSignals.h>
#include <SimulatorLib/SimSoftware.h>
#include <VFrame30/AppSignalController.h>
#include <VFrame30/PropertyNames.h>


namespace SimUi
{
	// MonitorView
	//
	SimSchemaView::SimSchemaView(SimSchemaManager* schemaManager, QWidget* parent) :
		VFrame30::ClientSchemaView(schemaManager, nullptr /*History navigation is not supported (now)*/, nullptr, parent),
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

		if (auto monitor = m_simulator->software().monitor(m_monitorId); monitor.has_value() == true)
		{
			setGlobalScript(monitor->globalScript());
			setMonitorBehavior(monitor->monitorBehavior());
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

		// Create global variable "equipment"
		//
		{
			Q_ASSERT(m_scriptEquipment);

			QJSValue jsObject = engine.newQObject(m_scriptEquipment.get());
			QJSEngine::setObjectOwnership(m_scriptEquipment.get(), QJSEngine::CppOwnership);

			engine.globalObject().setProperty(VFrame30::PropertyNames::scriptGlobalVariableEquipment, jsObject);
		}

		return;
	}

	void SimSchemaView::overrideSignalsChanged(QStringList /*addedAppSignalIds*/)
	{
		QStringList ids = m_simulator->overrideSignals().overrideSignalIds();

		setHighlightSignalIds(ids);

		return;
	}
} // namespace SimUi