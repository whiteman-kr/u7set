#pragma once

#include "../VFrame30/ClientSchemaView.h"
#include "SimSchemaManager.h"


class SimSchemaView : public VFrame30::ClientSchemaView
{
	Q_OBJECT

public:
	explicit SimSchemaView(SimSchemaManager* schemaManager, QWidget* parent = nullptr);
	virtual ~SimSchemaView();

public slots:
	void overrideSignalsChanged(QStringList addedAppSignalIds);	// Added or deleted signal

private:
	SimIdeSimulator* m_simulator = nullptr;
};



