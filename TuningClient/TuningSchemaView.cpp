#include "TuningSchemaView.h"
#include "Main.h"
#include "../VFrame30/PropertyNames.h"

TuningSchemaView::TuningSchemaView(TuningSchemaManager* schemaManager, QWidget* parent /*= nullptr*/)
	:  VFrame30::ClientSchemaView(schemaManager, nullptr, parent)
{

	QJSEngine* engine = jsEngine();

	if (engine == nullptr)
	{
		Q_ASSERT(engine);
		return ;
	}

	connect(schemaManager->configController(), &ConfigController::configurationArrived, this, &TuningSchemaView::configurationArrived);

	return;
}

VFrame30::DrawMode TuningSchemaView::drawMode() const
{
	return VFrame30::DrawMode::Monitor;
}

void TuningSchemaView::paintEvent(QPaintEvent* event)
{
	// It is possible that arrived configuration was not yet applied, it can happen in the very beginning,
	// when the schema was not created yet, but the configuration already received.
	//
	if (theConfigSettings.configurationId != m_configurationId)
	{
		configurationArrived(theConfigSettings);
	}

	return ClientSchemaView::paintEvent(event);
}

void TuningSchemaView::updateScriptGlobalVars(QJSEngine& engine)
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

	return;
}

void TuningSchemaView::configurationArrived(ConfigSettings configuration)
{
	m_configurationId = configuration.configurationId;

	setGlobalScript(configuration.scriptGlobal);
	setOnConfigurationArrivedScript(configuration.scriptConfigArrived);

	return;
}
