#pragma once

#include <VFrame30/ClientSchemaView.h>
#include "SimSchemaManager.h"

class SimIdeSimulator;

class SimSchemaView : public VFrame30::ClientSchemaView
{
	Q_OBJECT

public:
	explicit SimSchemaView(SimSchemaManager* schemaManager, QWidget* parent = nullptr);
	virtual ~SimSchemaView();

public:
	virtual VFrame30::DrawMode drawMode() const override;

	QString monitorId() const;
	void setMonitorId(QString equipmentId, bool emitUpdate);

protected:
	virtual void updateScriptGlobalVars(QJSEngine& engine) override;

public slots:
	void overrideSignalsChanged(QStringList addedAppSignalIds);	// Added or deleted signal

private:
	SimIdeSimulator* m_simulator = nullptr;
	QString m_monitorId;
};