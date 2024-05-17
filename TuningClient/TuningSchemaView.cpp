#include "TuningSchemaView.h"
#include "Main.h"
#include <VFrame30/PropertyNames.h>

TuningSchemaView::TuningSchemaView(TuningConfigController& configController, TuningSchemaManager& schemaManager, QWidget* parent /*= nullptr*/)
	:  VFrame30::ClientSchemaView(&schemaManager, nullptr, nullptr, parent),
	  m_configController(configController)
{
	m_app.setMainWindow(theApp.mainWindow());

	QJSEngine* engine = jsEngine();

	if (engine == nullptr)
	{
		Q_ASSERT(engine);
		return ;
	}

	connect(&m_configController, &TuningConfigController::configurationArrived, this, &TuningSchemaView::configurationArrived);

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
	if (m_configController.configuration().configurationId != m_configurationId)
	{
		configurationArrived(m_configController.configuration());
	}

	return ClientSchemaView::paintEvent(event);
}

void TuningSchemaView::updateScriptGlobalVars(QJSEngine& engine)
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

	return;
}

void TuningSchemaView::configurationArrived(TuningClientConfigSettings configuration)
{
	m_configurationId = configuration.configurationId;

	setGlobalScript(configuration.scriptGlobal);

	return;
}
