#include "MonitorView.h"
#include "MonitorSchemaManager.h"
#include "Settings.h"
#include "../lib/AppSignalManager.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/PropertyNames.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/TuningController.h"


// MonitorView
//
MonitorView::MonitorView(MonitorSchemaManager* schemaManager,
						 VFrame30::AppSignalController* appSignalController,
						 VFrame30::TuningController* tuningController,
						 QWidget* parent)
	: VFrame30::ClientSchemaView(schemaManager, parent)
{
	setAppSignalController(appSignalController);
	setTuningController(tuningController);

	qDebug() << Q_FUNC_INFO;

	Q_ASSERT(schemaManager);
	Q_ASSERT(schemaManager->monitorConfigController());

	connect(schemaManager->monitorConfigController(), &MonitorConfigController::configurationArrived, this, &MonitorView::configurationArrived);

	return;
}


MonitorView::~MonitorView()
{
	qDebug() << Q_FUNC_INFO;
	return;
}

void MonitorView::paintEvent(QPaintEvent* event)
{
	this->setInfoMode(theSettings.showItemsLabels());
	return ClientSchemaView::paintEvent(event);
}

void MonitorView::configurationArrived(ConfigSettings configuration)
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

	runScript(scriptValue, "run onConfigurationArrivedScript", true);

	return;
}

MonitorSchemaManager* MonitorView::monitorSchemaManager()
{
	MonitorSchemaManager* result = dynamic_cast<MonitorSchemaManager*>(schemaManager());
	Q_ASSERT(result);

	return result;
}

const MonitorSchemaManager* MonitorView::monitorSchemaManager() const
{
	const MonitorSchemaManager* result = dynamic_cast<const MonitorSchemaManager*>(schemaManager());
	Q_ASSERT(result);

	return result;
}


