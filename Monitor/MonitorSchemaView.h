#pragma once

#include "MonitorSchemaManager.h"
#include "ScriptMonitorApplication.h"
#include "../VFrame30/ClientSchemaView.h"

namespace VFrame30
{
	class ITimeStats;
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
							   VFrame30::ITimeStats* timeStats,
							   QWidget* parent = nullptr);
	virtual ~MonitorSchemaView() = default;

public:
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



