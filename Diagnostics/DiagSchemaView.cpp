#include "DiagSchemaView.h"
#include "DiagnosticsAppSettings.h"
#include "DiagnosticsMainWindow.h"
#include "DiagnosticsSchemaManager.h"
#include "Globals.h"

#include <VFrame30/AppSignalController.h>
#include <VFrame30/DrawParam.h>
#include <VFrame30/PropertyNames.h>

//
// DiagSchemaView
//
DiagSchemaView::DiagSchemaView(DiagnosticsSchemaManager* schemaManager,
							   VFrame30::ISchemaViewHistory* schemaViewHistory,
							   VFrame30::AppSignalController* appSignalController,
							   VFrame30::LogController* logController,
							   VFrame30::ITimeStats* timeStats,
							   QWidget* parent) :
	VFrame30::ClientSchemaView(schemaManager, schemaViewHistory, timeStats, parent)
{
	m_app.setMainWindow(theApp.mainWindow());

	setAppSignalController(appSignalController);
	setLogController(logController);

	Q_ASSERT(schemaManager);

	return;
}

VFrame30::DrawMode DiagSchemaView::drawMode() const
{
	return VFrame30::DrawMode::Monitor;
}

void DiagSchemaView::updateConfiguration(const DiagConfigSettings& configuration)
{
	qDebug() << "DiagSchemaView::configurationArrived()";

	m_configurationId = configuration.configurationId;

	// setMonitorBehavior(std::move(configuration.monitorBeahvior));

	// This will update GlobalScripts and reevaluate them.
	//
	setGlobalScript(configuration.globalScript);

	return;
}

void DiagSchemaView::paintEvent(QPaintEvent* event)
{
	// It is possible that arrived configuration was not yet applied, it can happen in the very beginning,
	// as the first tab page is created by timer in DiagnosticsCentralWidget::timerEvent, see comment there for
	// details.
	//
	if (int cid = diagSchemaManager()->configController().configurationId(); cid != m_configurationId)
	{
		updateConfiguration(diagSchemaManager()->configController().configuration());
	}

	setInfoMode(DiagnosticsAppSettings::instance().showItemsLabels());
	return ClientSchemaView::paintEvent(event);
}

void DiagSchemaView::updateScriptGlobalVars(QJSEngine& engine)
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

	return;
}

DiagnosticsSchemaManager* DiagSchemaView::diagSchemaManager()
{
	auto result = dynamic_cast<DiagnosticsSchemaManager*>(schemaManager());
	Q_ASSERT(result);

	return result;
}

const DiagnosticsSchemaManager* DiagSchemaView::diagSchemaManager() const
{
	auto result = dynamic_cast<const DiagnosticsSchemaManager*>(schemaManager());
	Q_ASSERT(result);

	return result;
}
