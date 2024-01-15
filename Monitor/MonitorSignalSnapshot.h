#pragma once

#include "../lib/Ui/DialogSignalSnapshot.h"
#include "MonitorCentralWidget.h"


class MonitorConfigController;
class MonitorSignalManager;


class MonitorDialogSignalSnapshot : public DialogSignalSnapshot
{
	Q_OBJECT

public:
	static MonitorDialogSignalSnapshot* createDialog(MonitorConfigController* configController,
													 MonitorSignalManager* monitorSignalManager,
													 MonitorCentralWidget* centralWidget);

private:
	explicit MonitorDialogSignalSnapshot(MonitorConfigController* configController,
										 IAppSignalManager* appSignalManager,
										 ISignalDataServer* signalDataServer,
										 const QString& projectName,
										 const QString& equipmentId,
										 QWidget *parent);

private:
	virtual std::vector<VFrame30::SchemaDetails> schemasDetails() override;
	virtual std::set<QString> schemaAppSignals(const QString& schemaStrId) override;

private:
	MonitorConfigController* m_configController = nullptr;
};

