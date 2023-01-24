#include "MonitorSchemaView.h"
#include "MonitorSchemaManager.h"
#include "MonitorAppSettings.h"
#include "../AppSignalLib/AppSignalManager.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/PropertyNames.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/TuningController.h"

//
// MonitorView
//
MonitorSchemaView::MonitorSchemaView(MonitorSchemaManager* schemaManager,
									 VFrame30::ISchemaViewHistory* schemaViewHistory,
									 VFrame30::AppSignalController* appSignalController,
									 VFrame30::TuningController* tuningController,
									 VFrame30::LogController* logController,
									 QWidget* parent)
	: VFrame30::ClientSchemaView(schemaManager, schemaViewHistory, parent)
{
	setAppSignalController(appSignalController);
	setTuningController(tuningController);
	setLogController(logController);

	Q_ASSERT(schemaManager && schemaManager->monitorConfigController());

	connect(schemaManager->monitorConfigController(), &MonitorConfigController::configurationArrived, this, &MonitorSchemaView::configurationArrived);

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
	if (int cid = monitorSchemaManager()->monitorConfigController()->configurationId();
		cid != m_configurationId)
	{
		configurationArrived(monitorSchemaManager()->monitorConfigController()->configuration());
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
		QJSValue jsApp = engine.newQObject(&theApp);
		QQmlEngine::setObjectOwnership(&theApp, QQmlEngine::CppOwnership);

		engine.globalObject().setProperty(VFrame30::PropertyNames::scriptGlobalVariableApp, jsApp);
	}

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

void MonitorSchemaView::configurationArrived(ConfigSettings configuration)
{
	m_configurationId = configuration.configurationId;

	setGlobalScript(configuration.globalScript);
	setOnConfigurationArrivedScript(configuration.onConfigurationArrivedScript);

	setMonitorBehavior(std::move(configuration.monitorBeahvior));

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


