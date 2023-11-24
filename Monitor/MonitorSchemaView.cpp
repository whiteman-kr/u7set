#include "MonitorSchemaView.h"
#include "MonitorSchemaManager.h"
#include "MonitorAppSettings.h"
#include "Globals.h"
#include "../VFrame30/DrawParam.h"
#include "../VFrame30/PropertyNames.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/TuningController.h"
#include "MonitorMainWindow.h"

//
// MonitorView
//
MonitorSchemaView::MonitorSchemaView(MonitorSchemaManager* schemaManager,
									 VFrame30::ISchemaViewHistory* schemaViewHistory,
									 VFrame30::AppSignalController* appSignalController,
									 VFrame30::LogController* logController,
									 ITimeStats* timeStats,
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

	connect(&schemaManager->monitorConfigController(), &MonitorConfigController::configurationArrived, this, &MonitorSchemaView::configurationArrived);

	// Updates scripts
	//
	configurationArrived(monitorSchemaManager()->monitorConfigController().configuration());

	return;
}

bool MonitorSchemaView::saveSchemaToPdf(const QString& fileName)
{
	if (schema() == nullptr)
	{
		Q_ASSERT(schema());
		return false;
	}

	// --
	//
	QPdfWriter pdfWriter(fileName);

	pdfWriter.setTitle(schema()->caption());

	QPageSize pageSize;
	double pageWidth = schema()->docWidth();
	double pageHeight = schema()->docHeight();

	if (schema()->unit() == SchemaUnit::Inch)
	{
		pageSize = QPageSize(QSizeF(pageWidth, pageHeight), QPageSize::Inch);
	}
	else
	{
		assert(schema()->unit() == SchemaUnit::Display);
		pageSize = QPageSize(QSize(static_cast<int>(pageWidth), static_cast<int>(pageHeight)));

		pdfWriter.setResolution(72);	// 72 is from enum QPageLayout::Unit help,
										// QPageLayout::Point	1	1/!!! 72th !!!! of an inch
	}

	pdfWriter.setPageSize(pageSize);
	pdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0));

	// --
	//
	QPainter p(&pdfWriter);
	VFrame30::CDrawParam drawParam(&p, this, schema()->gridSize(), schema()->pinGridStep(), schema()->unit());

	drawParam.setInfoMode(false);
	drawParam.setPdfMode(true);

	// Calc size
	//
	int widthInPixel = schema()->GetDocumentWidth(pdfWriter.resolution(), 100.0);		// Export 100% zoom
	int heightInPixel = schema()->GetDocumentHeight(pdfWriter.resolution(), 100.0);		// Export 100% zoom

	// Clear device
	//
	p.fillRect(QRectF(0, 0, widthInPixel + 1, heightInPixel + 1), QColor(0xB0, 0xB0, 0xB0));
	p.setRenderHint(QPainter::Antialiasing);

	// Ajust QPainter
	//
	Ajust(&p, schema()->unit(), 0, 0, 100.0);			// Export 100% zoom

	// Draw Schema
	//
	QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());

	schema()->Draw(&drawParam, clipRect);

	// Ending
	//

	return true;
}

bool MonitorSchemaView::saveSchemaToPng(const QString& fileName)
{
	if (schema() == nullptr)
	{
		Q_ASSERT(schema());
		return false;
	}

	QPageSize pageSize;
	double pageWidth = schema()->docWidth();
	double pageHeight = schema()->docHeight();

	if (schema()->unit() == SchemaUnit::Inch)
	{
		pageSize = QPageSize(QSizeF(pageWidth, pageHeight), QPageSize::Inch);
	}
	else
	{
		assert(schema()->unit() == SchemaUnit::Display);
		pageSize = QPageSize(QSize(static_cast<int>(pageWidth), static_cast<int>(pageHeight)));
	}

	// Calc size
	//
	const int resolution = 300;	// Image resolution is 300 dpi

	int widthInPixel = schema()->GetDocumentWidth(resolution, 100.0);		// Export 100% zoom
	int heightInPixel = schema()->GetDocumentHeight(resolution, 100.0);		// Export 100% zoom

	// --
	//
	QImage image(QSize(widthInPixel, heightInPixel), QImage::Format_RGB32);

	QPainter p(&image);
	VFrame30::CDrawParam drawParam(&p, this, schema()->gridSize(), schema()->pinGridStep(), schema()->unit());

	drawParam.setInfoMode(false);
	drawParam.setPdfMode(true);

	// Clear device
	//
	p.fillRect(QRectF(0, 0, widthInPixel + 1, heightInPixel + 1), QColor(0xB0, 0xB0, 0xB0));
	p.setRenderHint(QPainter::Antialiasing);

	// Ajust QPainter
	//
	Ajust(&p, schema()->unit(), 0, 0, (double)resolution / image.logicalDpiX() * 100.0);			// Export 100% zoom

	// Draw Schema
	//
	QRectF clipRect(0, 0, schema()->docWidth(), schema()->docHeight());

	schema()->Draw(&drawParam, clipRect);

	// Saving
	//
	if (image.save(fileName) == false)
	{
		return false;
	}
	return true;
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
	if (int cid = monitorSchemaManager()->monitorConfigController().configurationId();
		cid != m_configurationId)
	{
		configurationArrived(monitorSchemaManager()->monitorConfigController().configuration());
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
		QQmlEngine::setObjectOwnership(&m_app, QQmlEngine::CppOwnership);

		engine.globalObject().setProperty(VFrame30::PropertyNames::scriptGlobalVariableApp, jsApp);
	}

	// create global variable "tuning"
	//
	{
		QJSValue jsTuning = engine.newQObject(m_tuningController.get());
		QQmlEngine::setObjectOwnership(m_tuningController.get(), QQmlEngine::CppOwnership);

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
	qDebug() << "MonitorSchemaView::configurationArrived()";

	m_configurationId = configuration.configurationId;

	setMonitorBehavior(std::move(configuration.monitorBeahvior));

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


