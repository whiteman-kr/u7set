#pragma once

#include "../VFrame30/ClientSchemaView.h"
#include "../lib/ITimeStats.h"
#include "ScriptDiagnosticsApplication.h"
#include "DiagConfigController.h"

class DiagnosticsSchemaManager;

// #include "../VFrame30/AppSignalController.h"
////
////namespace VFrame30
////{
////	class AppSignalController;
////	class TuningController;
////}

class DiagSchemaView : public VFrame30::ClientSchemaView
{
	Q_OBJECT

public:
	explicit DiagSchemaView(DiagnosticsSchemaManager* schemaManager,
							VFrame30::ISchemaViewHistory* schemaViewHistory,
							// -- VFrame30::AppSignalController* appSignalController,
							VFrame30::LogController* logController,
							ITimeStats* timeStats,
							QWidget* parent = nullptr);
	virtual ~DiagSchemaView() = default;

public:
	virtual VFrame30::DrawMode drawMode() const override;

protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void updateScriptGlobalVars(QJSEngine& engine) override;

public slots:
	void configurationArrived(DiagConfigSettings configuration);

	DiagnosticsSchemaManager* diagSchemaManager();
	const DiagnosticsSchemaManager* diagSchemaManager() const;

	// Data
	//
private:
	int m_configurationId = -1;
	ScriptDiagnosticsApplication m_app;
};
