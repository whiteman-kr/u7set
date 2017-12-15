#pragma once

#include "../lib/Service.h"
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
							   const QString& serviceName,
							   int& argc,
							   char** argv,
							   std::shared_ptr<CircularLogger> logger);

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const;

public slots:
	void onBuildPathChanged(QString newBuildPath);

signals:
	void renameWorkBuildToBackupExcept(QString workDirectoryToLeave);

private:
	virtual void initCmdLineParser() override;
	virtual void loadSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	void startCfgServerThread(const QString& buildPath);
	void stopCfgServerThread();

	void startCfgCheckerThread();
	void stopCfgCheckerThread();

	void startUdpThreads();
	void stopUdpThreads();

private:
	static const char* const SETTING_EQUIPMENT_ID;
	static const char* const SETTING_AUTOLOAD_BUILD_PATH;
	static const char* const SETTING_CLIENT_REQUEST_IP;
	static const char* const SETTING_WORK_DIRECTORY;

	std::shared_ptr<CircularLogger> m_logger;
	UdpSocketThread* m_infoSocketThread = nullptr;
	Tcp::ServerThread* m_cfgServerThread = nullptr;

	CfgCheckerWorker* m_cfgCheckerWorker = nullptr;
	SimpleThread* m_cfgCheckerThread = nullptr;

	// settings
	//
	QString m_equipmentID;
	QString m_autoloadBuildPath;
	QString m_clientIPStr;
	QString m_workDirectory;

	HostAddressPort m_clientIP;
};
