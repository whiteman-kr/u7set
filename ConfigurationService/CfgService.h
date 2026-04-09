#pragma once

#include <ServiceLib/Service.h>
#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/Tcp.h"
#include "CfgChecker.h"
#include "GrpcCfgServer.h"

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
							   int argc,
							   char** argv,
							   std::shared_ptr<CircularLogger> logger);

	ConfigurationServiceWorker(const ConfigurationServiceWorker* worker);

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const override;

public slots:
	void onBuildPathChanged(QString newBuildPath);

signals:
	void renameWorkBuildToBackupExcept(QString workDirectoryToLeave);

private:
	virtual void initServiceSpecificCmdLineArgs() override;
	virtual void loadServiceSpecificSettings() override;

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
	UdpSocketThread* m_infoSocketThread = nullptr;
//	Tcp::ListenerThread* m_cfgServerThread = nullptr;

	using GrpcCfgServerUPtr = std::unique_ptr<GrpcCfgServer>;

	std::vector<GrpcCfgServerUPtr> m_grpcCfgServers;

	CfgCheckerWorker* m_cfgCheckerWorker = nullptr;
	SimpleThread* m_cfgCheckerThread = nullptr;

	// settings
	//
	QString m_autoloadBuildPath;
	QString m_workDirectory;
	bool m_checkHostname = false;

	CfgServiceSettings m_cfgServiceSettings;
};
