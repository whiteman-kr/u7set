#pragma once

#include "../lib/Service.h"
#include "../lib/SoftwareSettings.h"
#include "CfgControlServer.h"

// ------------------------------------------------------------------------------------
//
// ConfigurationServiceWorker class declaration
//
// ------------------------------------------------------------------------------------

class ConfigurationServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	ConfigurationServiceWorker(const SoftwareInfo& softwareInfo,
							   const QString& serviceInstanceName,
							   int& argc,
							   char** argv,
							   std::shared_ptr<CircularLogger> logger);

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const;

public slots:
	void onBuildPathChanged(QString newBuildPath);

signals:
	void renameWorkBuildToBackupExcept(QString workDirectoryToLeave);

private:
	virtual void initCmdLineParser() override;
	virtual void loadSettings() override;

	bool loadCfgServiceSettings(const QString& buildPath);

	virtual void initialize() override;
	virtual void shutdown() override;

	void startCfgServerThread(const QString& buildPath);
	void stopCfgServerThread();

	void startCfgCheckerThread();
	void stopCfgCheckerThread();

	void startUdpThreads();
	void stopUdpThreads();

	E::SoftwareRunMode getSoftwareRunMode(QString runModeStr);

private:
	std::shared_ptr<CircularLogger> m_logger;
	UdpSocketThread* m_infoSocketThread = nullptr;
	Tcp::ServerThread* m_cfgServerThread = nullptr;

	CfgCheckerWorker* m_cfgCheckerWorker = nullptr;
	SimpleThread* m_cfgCheckerThread = nullptr;

	// settings
	//
	QString m_autoloadBuildPath;
	QString m_clientIPStr;
	QString m_workDirectory;

	HostAddressPort m_clientIP;

	CfgServiceSettings m_cfgServiceSettings;
};
