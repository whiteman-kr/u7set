#ifndef MONITORSIGNALINFO_H
#define MONITORSIGNALINFO_H

#include "../lib/Ui/DialogSignalInfo.h"

class MonitorCentralWidget;
class MonitorConfigController;
class TcpSignalClient;

class MonitorSignalInfo : public DialogSignalInfo
{
public:
	static bool showDialog(QString appSignalId, MonitorConfigController* configController, TcpSignalClient* tcpSignalClient, MonitorCentralWidget* centralWidget);

private:
	MonitorSignalInfo(const AppSignalParam& signal,
					  MonitorConfigController* configController,
					  IAppSignalManager* appSignalManager,
					  VFrame30::TuningController* tuningController,
					  bool tuningEnabled,
					  MonitorCentralWidget* centralWidget);

private:
	virtual QStringList schemasByAppSignalId(const QString& appSignalId) override;
	virtual void setSchema(QString schemaId, QStringList highlightIds) override;

private:
	MonitorConfigController* m_configController = nullptr;

	MonitorCentralWidget* m_centralWidget = nullptr;

};


#endif // MONITORSIGNALINFO_H
