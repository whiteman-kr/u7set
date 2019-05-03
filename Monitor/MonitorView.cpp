#include "MonitorView.h"
#include "MonitorSchemaManager.h"
#include "../lib/AppSignalManager.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/PropertyNames.h"


// MonitorView
//
MonitorView::MonitorView(MonitorSchemaManager* schemaManager, QWidget *parent)
	: VFrame30::ClientSchemaView(schemaManager, parent)
{
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

void MonitorView::configurationArrived(ConfigSettings configuration)
{
	QJSEngine* engine = jsEngine();

	if (engine == nullptr)
	{
		Q_ASSERT(engine);
		return ;
	}

	QJSValue scriptValue = evaluateScript(configuration.onConfigurationArrivedScript, true);
	if (scriptValue.isError() == true ||
		scriptValue.isUndefined() == true)
	{
		return;
	}

	runScript(scriptValue, true);

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


