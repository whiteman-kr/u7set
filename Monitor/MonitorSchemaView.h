#pragma once

#include "../VFrame30/ClientSchemaView.h"
#include "MonitorSchemaManager.h"

namespace VFrame30
{
	class AppSignalController;
	class TuningController;
}

class MonitorSchemaView : public VFrame30::ClientSchemaView
{
	Q_OBJECT

public:
	explicit MonitorSchemaView(MonitorSchemaManager* schemaManager,
						 VFrame30::AppSignalController* appSignalController,
						 VFrame30::TuningController* tuningController,
						 VFrame30::LogController* logController,
						 QWidget* parent = nullptr);
	virtual ~MonitorSchemaView();

protected:
	virtual void paintEvent(QPaintEvent* event) override;

	// Properties
	//
public:

public slots:
	void configurationArrived(ConfigSettings configuration);

	MonitorSchemaManager* monitorSchemaManager();
	const MonitorSchemaManager* monitorSchemaManager() const;

	// Data
	//
private:
};



