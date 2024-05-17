#pragma once

#include <SchemaClientLib/DialogSignalSnapshot.h>

class MonitorCentralWidget;
class MonitorConfigController;

namespace ClientLib
{
	class AppSignalManager;
}

namespace AppSignalLists
{
	class AppSignalListSet;
}

class MonitorDialogSignalSnapshot : public SchemaClientLib::DialogSignalSnapshot
{
	Q_OBJECT

public:
	static MonitorDialogSignalSnapshot* createDialog(MonitorConfigController* configController,
													 ClientLib::AppSignalManager* monitorAppSignalManager,
													 AppSignalLists::AppSignalListSet* appSignalListSet,
													 MonitorCentralWidget* centralWidget);

private:
	explicit MonitorDialogSignalSnapshot(MonitorConfigController* configController,
										 IAppSignalManager* appSignalManager,
										 ISignalDataServer* signalDataServer,
										 AppSignalLists::AppSignalListSet* appSignalListSet,
										 const QString& projectName,
										 const QString& equipmentId,
										 QWidget *parent);

private:
	virtual std::vector<VFrame30::SchemaDetails> schemasDetails() override;
	virtual std::set<QString> schemaAppSignals(const QString& schemaStrId) override;

private:
	MonitorConfigController* m_configController = nullptr;
};

