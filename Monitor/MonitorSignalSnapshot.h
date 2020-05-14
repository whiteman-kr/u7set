#ifndef MONITORDIALOGSIGNALSNAPSHOT_H
#define MONITORDIALOGSIGNALSNAPSHOT_H

#include "../lib/Ui/DialogSignalSnapshot.h"

#include "TcpSignalClient.h"
#include "MonitorCentralWidget.h"

class MonitorDialogSignalSnapshot : public DialogSignalSnapshot
{
public:
	static bool showDialog(MonitorConfigController *configController,
						   TcpSignalClient* tcpSignalClient,
						   AppSignalManager* appSignalManager,
						   QString projectName,
						   QString softwareEquipmentId,
						   MonitorCentralWidget* centralWidget);

private:
	explicit MonitorDialogSignalSnapshot(MonitorConfigController *configController,
								  AppSignalManager* appSignalManager,
								  QString projectName,
								  QString softwareEquipmentId,
								  QWidget *parent);

private:
	virtual std::vector<VFrame30::SchemaDetails> schemasDetails() override;
	virtual std::set<QString> schemaAppSignals(const QString& schemaStrId) override;

private:
	MonitorConfigController* m_configController = nullptr;
};

#endif // MONITORDIALOGSIGNALSNAPSHOT_H
