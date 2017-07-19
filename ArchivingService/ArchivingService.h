#pragma once

#include "../lib/Service.h"
#include "../lib/CfgServerLoader.h"
#include "../lib/ServiceSettings.h"
#include "../lib/Queue.h"

#include "TcpAppDataServer.h"
#include "ArchWriteThread.h"
#include "TcpArchiveRequestsServer.h"

class ArchivingServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	ArchivingServiceWorker(const QString &serviceName,
						   int &argc,
						   char **argv,
						   const VersionInfo &versionInfo,
						   std::shared_ptr<CircularLogger> logger);
	~ArchivingServiceWorker();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const override;

private:
	virtual void initCmdLineParser() override;
	virtual void loadSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void clearConfiguration();
	void applyNewConfiguration();

	void runArchWriteThread();
	void runAppDataServerThread();
	void runArchiveRequestsServerThread();

	void stopArchWriteThread();
	void stopAppDataServerThread();
	void stopArchiveRequestsServerThread();

	bool readConfiguration(const QByteArray& fileData);
	bool loadConfigurationFromFile(const QString& fileName);

	bool initArchSignalsMap(const QByteArray& fileData);


private slots:
	void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

private:
	QString m_equipmentID;
	QString m_cfgServiceIP1Str;
	QString m_cfgServiceIP2Str;

	HostAddressPort m_cfgServiceIP1;
	HostAddressPort m_cfgServiceIP2;

	std::shared_ptr<CircularLogger> m_logger;

	QSettings m_settings;

	ArchivingServiceSettings m_cfgSettings;
	QString m_projectID;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	//

	TcpAppDataServerThread* m_tcpAppDataServerThread = nullptr;
	ArchWriteThread* m_archWriteThread = nullptr;
	TcpArchiveRequestsServerThread* m_tcpArchiveRequestsServerThread = nullptr;

	AppSignalStatesQueue m_saveStatesQueue;

	QHash<Hash, bool> m_archSignals;
};

