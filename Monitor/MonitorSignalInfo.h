#ifndef MONITORSIGNALINFO_H
#define MONITORSIGNALINFO_H

#include "MonitorSignalManager.h"
#include "../lib/Ui/DialogSignalInfo.h"
#include "../lib/ISignalDataServer.h"

class MonitorCentralWidget;
class MonitorConfigController;

class MonitorSignalInfo : public DialogSignalInfo
{
	Q_OBJECT
public:
	static bool showDialog(QString appSignalId,
						   MonitorSignalManager* appSignalManager,
						   ITuningSignalManager& tuningSignalManager,
						   ITuningConnection& tuningConnection,
						   ITuningAuthorization& tuningAuthorization,
						   MonitorConfigController* configController,
						   MonitorCentralWidget* centralWidget);

private:
	MonitorSignalInfo(const AppSignalParam& signal,
					  MonitorConfigController* configController,
					  IAppSignalManager* appSignalManager,
					  ISignalDataServer* signalDataServer,
					  ITuningSignalManager& tuningSignalManager,
					  ITuningConnection& tuningConnection,
					  ITuningAuthorization& tuningAuthorization,
					  bool tuningEnabled,
					  MonitorCentralWidget* centralWidget);

private slots:
	void onSignalParamAndUnitsArrived();

private:
	virtual QStringList schemasByAppSignalId(const QString& appSignalId) override;
	virtual void setSchema(QString schemaId, QStringList highlightIds) override;
	virtual std::optional<AppSignal> getSignalExt(const AppSignalParam& appSignalParam) override;

private:
	MonitorConfigController* m_configController = nullptr;

	MonitorCentralWidget* m_centralWidget = nullptr;
};


#endif // MONITORSIGNALINFO_H
