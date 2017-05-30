#pragma once

#include "../lib/Service.h"
#include "../lib/CfgServerLoader.h"

// ------------------------------------------------------------------------------------
//
// ConfigurationServiceWorker class declaration
//
// ------------------------------------------------------------------------------------

class ConfigurationServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	ConfigurationServiceWorker(const QString& serviceName,
							   int& argc,
							   char** argv,
							   const VersionInfo& versionInfo,
							   std::shared_ptr<CircularLogger> logger);

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const;

public slots:
	void onInformationRequest(UdpRequest request);
	void onBuildPathChanged(QString newBuildPath);

signals:
	void ackInformationRequest(UdpRequest request);
	void renameWorkBuildToBackupExcept(QString workDirectoryToLeave);

private:
	virtual void initCmdLineParser() override;
	virtual void processCmdLineSettings() override;
	virtual void loadSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	void startCfgServerThread(const QString& buildPath);
	void stopCfgServerThread();

	void startCfgCheckerThread();
	void stopCfgCheckerThread();

	void startUdpThreads();
	void stopUdpThreads();

	void onGetInfo(UdpRequest& request);
	void onGetSettings(UdpRequest& request);
	void onSetSettings(UdpRequest& request);

private:
	std::shared_ptr<CircularLogger> m_logger;
	UdpSocketThread* m_infoSocketThread = nullptr;
	Tcp::ServerThread* m_cfgServerThread = nullptr;

	SimpleThread* m_cfgCheckerThread = nullptr;

	// settings
	//
	QString m_equipmentID;
	QString m_autoloadBuildPath;
	QString m_clientIPStr;
	QString m_workDirectory;

	HostAddressPort m_clientIP;
};
