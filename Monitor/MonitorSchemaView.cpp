#include "MonitorSchemaView.h"
#include "MonitorSchemaManager.h"
#include "MonitorAppSettings.h"
#include "../lib/AppSignalManager.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/PropertyNames.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/TuningController.h"


// MonitorView
//
MonitorSchemaView::MonitorSchemaView(MonitorSchemaManager* schemaManager,
						 VFrame30::AppSignalController* appSignalController,
						 VFrame30::TuningController* tuningController,
						 VFrame30::LogController* logController,
						 QWidget* parent)
	: VFrame30::ClientSchemaView(schemaManager, parent)
{
	setAppSignalController(appSignalController);
	setTuningController(tuningController);
	setLogController(logController);

	Q_ASSERT(schemaManager);
	Q_ASSERT(schemaManager->monitorConfigController());

	connect(schemaManager->monitorConfigController(), &MonitorConfigController::configurationArrived, this, &MonitorSchemaView::configurationArrived);

	return;
}


MonitorSchemaView::~MonitorSchemaView()
{
	return;
}

void MonitorSchemaView::paintEvent(QPaintEvent* event)
{
	this->setInfoMode(MonitorAppSettings::instance().showItemsLabels());
	return ClientSchemaView::paintEvent(event);
}

void MonitorSchemaView::configurationArrived(ConfigSettings configuration)
{
	// --
	//
	setMonitorBehavior(std::move(configuration.monitorBeahvior));

	// --
	//
	QJSEngine* engine = jsEngine();

	if (engine == nullptr)
	{
		Q_ASSERT(engine);
		return ;
	}

	reEvaluateGlobalScript();

	QJSValue scriptValue = evaluateScript(configuration.onConfigurationArrivedScript, "evaluate onConfigurationArrivedScript", true);
	if (scriptValue.isError() == true ||
		scriptValue.isUndefined() == true)
	{
		return;
	}

	// --
	//
	runScript(scriptValue, "run onConfigurationArrivedScript", true);

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


