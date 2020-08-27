#include "TuningSchemaView.h"
#include "TuningSchemaWidget.h"
#include "MainWindow.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/MonitorSchema.h"

TuningSchemaView::TuningSchemaView(TuningSchemaManager* schemaManager, QWidget* parent /*= nullptr*/)
	:  VFrame30::ClientSchemaView(schemaManager, parent)
{

	// --
	//
	QJSEngine* engine = jsEngine();

	if (engine == nullptr)
	{
		Q_ASSERT(engine);
		return ;
	}

	QJSValue scriptValue = evaluateScript(schemaManager->configurationArrivedScript(), "evaluate configurationArrivedScript", true);
	if (scriptValue.isError() == true ||
		scriptValue.isUndefined() == true)
	{
		return;
	}

	runScript(scriptValue, "run configurationArrivedScript", true);
}

TuningSchemaView::~TuningSchemaView()
{
}

