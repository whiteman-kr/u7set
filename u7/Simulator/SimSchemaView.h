#pragma once

#include "../VFrame30/ClientSchemaView.h"
#include "SimSchemaManager.h"


class SimSchemaView : public VFrame30::ClientSchemaView
{
	Q_OBJECT

public:
	explicit SimSchemaView(SimSchemaManager* schemaManager, QWidget* parent = nullptr);
	virtual ~SimSchemaView();

	// Properties
	//
public:

	// Data
	//
private:

};



