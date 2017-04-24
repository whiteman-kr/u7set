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
	void onBuildPathChanged(const QString& newBuildPath);

signals:
	void ackInformationRequest(UdpRequest request);

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

// ------------------------------------------------------------------------------------
//
// CfgCheckerWorker class declaration
//
// ------------------------------------------------------------------------------------

class CfgCheckerWorker : public SimpleThreadWorker
{
	Q_OBJECT

public:
	CfgCheckerWorker(const QString& workFolder,
					 const QString& autoloadBuildFolder,
					 int checkNewBuildInterval,
					 std::shared_ptr<CircularLogger> logger);

	QString getFileHash(const QString& filePath);
	bool copyPath(const QString& src, const QString& dst);
	bool checkBuild(const QString& buildDirectoryPath);

signals:
	void buildPathChanged(const QString& newBuildPath);

public slots:
	void updateBuildXml();

protected:
	void onThreadStarted();
	//void onThreadFinished();

private:
	QString m_workFolder;
	QString m_autoloadBuildFolder;
	QDateTime m_lastBuildXmlModifyTime;
	QString m_lastBuildXmlHash;
	int m_checkNewBuildInterval;

	std::shared_ptr<CircularLogger> m_logger;
};

