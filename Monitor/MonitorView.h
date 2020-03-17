#pragma once

#include "../VFrame30/ClientSchemaView.h"
#include "MonitorSchemaManager.h"

namespace VFrame30
{
	class AppSignalController;
	class TuningController;
}

class MonitorView : public VFrame30::ClientSchemaView
{
	Q_OBJECT

public:
	explicit MonitorView(MonitorSchemaManager* schemaManager,
						 VFrame30::AppSignalController* appSignalController,
						 VFrame30::TuningController* tuningController,
						 QWidget* parent = nullptr);
	virtual ~MonitorView();

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



