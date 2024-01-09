#pragma once

#include "MonitorSchemaManager.h"
#include "ScriptMonitorApplication.h"
#include "../VFrame30/ClientSchemaView.h"
#include "../VFrame30/AppSignalController.h"
#include "../lib/ITimeStats.h"


namespace VFrame30
{
	class AppSignalController;
	class TuningController;
}

class MonitorSchemaView : public VFrame30::ClientSchemaView
{
	Q_OBJECT

public:
	MonitorSchemaView() = delete;
	explicit MonitorSchemaView(MonitorSchemaManager* schemaManager,
							   VFrame30::ISchemaViewHistory* schemaViewHistory,
							   VFrame30::AppSignalController* appSignalController,
							   VFrame30::LogController* logController,
							   ITimeStats* timeStats,
							   QWidget* parent = nullptr);
	virtual ~MonitorSchemaView() = default;

public:
	bool saveSchemaToPdf(const QString& fileName);	// Export schema to PDF or PNG
	bool saveSchemaToPng(const QString& fileName);	// Export schema to PDF or PNG

	virtual VFrame30::DrawMode drawMode() const override;

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void updateScriptGlobalVars(QJSEngine& engine) override;

public slots:
	void configurationArrived(MonitorConfigSettings configuration);

	MonitorSchemaManager* monitorSchemaManager();
	const MonitorSchemaManager* monitorSchemaManager() const;

	// Data
	//
private:
	int m_configurationId = -1;
	ScriptMonitorApplication m_app;
};



