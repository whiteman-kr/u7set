#include "MonitorSchemaView.h"
#include "Globals.h"
#include "MonitorAppSettings.h"
#include "MonitorMainWindow.h"
#include "MonitorSchemaManager.h"

#include <HardwareLib/ScriptEquipment.h>
#include <VFrame30/AppSignalController.h>
#include <VFrame30/ITimeStats.h>
#include <VFrame30/PropertyNames.h>


//
// MonitorView
//
MonitorSchemaView::MonitorSchemaView(MonitorSchemaManager* schemaManager,
									 VFrame30::ISchemaViewHistory* schemaViewHistory,
									 VFrame30::AppSignalController* appSignalController,
									 VFrame30::LogController* logController,
									 VFrame30::ITimeStats* timeStats,
									 QWidget* parent)
	: VFrame30::ClientSchemaView(schemaManager, schemaViewHistory, timeStats, parent)
{
	m_app.setMainWindow(theApp.mainWindow());

	setAppSignalController(appSignalController);
	setTuningController(theApp.mainWindow()->tuningSignalManager(),
						theApp.mainWindow()->tuningConnection(),
						theApp.mainWindow()->tuningAuthorization());
	setLogController(logController);

	Q_ASSERT(schemaManager);

	connect(&schemaManager->configController(), &MonitorConfigController::configurationArrived, this, &MonitorSchemaView::configurationArrived);

	return;
}

VFrame30::DrawMode MonitorSchemaView::drawMode() const
{
	return VFrame30::DrawMode::Monitor;
}

void MonitorSchemaView::paintEvent(QPaintEvent* event)
{
	// It is possible that arrived configuration was not yet applied, it can happen in the very beginning,
	// as the first tab page is created by timer in MonitorCentralWidget::timerEvent, see comment there for
	// details.
	//
	if (int cid = monitorSchemaManager()->configController().configurationId();
		cid != m_configurationId)
	{
		configurationArrived(monitorSchemaManager()->configController().configuration());
	}

	setInfoMode(MonitorAppSettings::instance().showItemsLabels());
	return ClientSchemaView::paintEvent(event);
}

void MonitorSchemaView::updateScriptGlobalVars(QJSEngine& engine)
{
	VFrame30::ClientSchemaView::updateScriptGlobalVars(engine);

	// create global variable "app"
	//
	{
		QJSValue jsApp = engine.newQObject(&m_app);
		QJSEngine::setObjectOwnership(&m_app, QJSEngine::CppOwnership);

		engine.globalObject().setProperty(VFrame30::PropertyNames::scriptGlobalVariableApp, jsApp);
	}

	// create global variable "tuning"
	//
	{
		QJSValue jsTuning = engine.newQObject(m_tuningController.get());
		QJSEngine::setObjectOwnership(m_tuningController.get(), QJSEngine::CppOwnership);

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

void MonitorSchemaView::configurationArrived(MonitorConfigSettings configuration)
{
	qDebug() << "MonitorSchemaView::configurationArrived()";

	m_configurationId = configuration.configurationId;

	setMonitorBehavior(std::move(configuration.monitorBehavior));

	m_scriptEquipment->setRoot(configuration.equipment);

	// This will update GlobalScripts and reevaluate them.
	//
	setGlobalScript(configuration.globalScript);

	// updateConfiguration resets schema, which triggers after create scripts, which can require GlobalScript.
	// At this point GlobalScript is considered evaluated.
	//
	monitorSchemaManager()->updateConfiguration(configuration);
	return;
}

MonitorSchemaManager* MonitorSchemaView::monitorSchemaManager()
{
	MonitorSchemaManager* result = dynamic_cast<MonitorSchemaManager*>(schemaManager());
	Q_ASSERT(result);

	return result;
}

const MonitorSchemaManager* MonitorSchemaView::monitorSchemaManager() const
{
	const MonitorSchemaManager* result = dynamic_cast<const MonitorSchemaManager*>(schemaManager());
	Q_ASSERT(result);

	return result;
}


