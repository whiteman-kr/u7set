#pragma once

#include <QJSEngine>
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

	// Data
	//
private:

};



