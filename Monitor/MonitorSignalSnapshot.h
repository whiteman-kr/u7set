#pragma once

#include <SchemaClientLib/DialogSignalSnapshot.h>

class MonitorCentralWidget;
class MonitorConfigController;

namespace ClientLib
{
	class AppSignalManager;
}

class MonitorDialogSignalSnapshot : public SchemaClientLib::DialogSignalSnapshot
{
	Q_OBJECT

public:
	static MonitorDialogSignalSnapshot* createDialog(MonitorConfigController* configController,
													 ClientLib::AppSignalManager* monitorAppSignalManager,
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

