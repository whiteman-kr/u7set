#pragma once

#include "../VFrame30/ClientSchemaView.h"
#include "MonitorSchemaManager.h"


class MonitorView : public VFrame30::ClientSchemaView
{
	Q_OBJECT

public:
	explicit MonitorView(MonitorSchemaManager* schemaManager, QWidget* parent = nullptr);
	virtual ~MonitorView();

	// Properties
	//
public:

public slots:
	void configurationArrived(ConfigSettings configuration);

	// Data
	//
private:

};



