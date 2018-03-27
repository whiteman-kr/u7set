#pragma once

#include "../lib/Service.h"
#include "../lib/CfgServerLoader.h"
#include "../lib/ServiceSettings.h"
#include "../lib/Queue.h"

#include "TcpAppDataServer.h"
#include "Archive.h"
#include "ArchWriteThread.h"
#include "TcpArchRequestsServer.h"
#include "ArchRequestThread.h"

class ArchivingServiceWorker : public ServiceWorker
{
	Q_OBJECT

public:
	ArchivingServiceWorker(const SoftwareInfo& softwareInfo,
						   const QString &serviceName,
						   int &argc,
						   char **argv,
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

	void runAllThreads();
	void stopAllThread();

	void createArchive();
	void deleteArchive();

	void runArchWriteThread();
	void stopArchWriteThread();

	void runTcpAppDataServerThread();
	void stopTcpAppDataServerThread();

	void runTcpArchRequestsServerThread();
	void stopTcpArchiveRequestsServerThread();

	void runArchRequestThread();
	void stopArchRequestThread();

	bool readConfiguration(const QByteArray& fileData);
	bool loadConfigurationFromFile(const QString& fileName);

	bool initArchSignalsMap(const QByteArray& fileData);

private slots:
	void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

private:
	std::shared_ptr<CircularLogger> m_logger;

	QSettings m_settings;

	ArchivingServiceSettings m_cfgSettings;
	QString m_projectID;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	//

	TcpAppDataServerThread* m_tcpAppDataServerThread = nullptr;
	TcpArchiveRequestsServerThread* m_tcpArchiveRequestsServerThread = nullptr;

	ArchWriteThread* m_archWriteThread = nullptr;
	ArchRequestThread* m_archRequestThread = nullptr;

	AppSignalStatesQueue m_saveStatesQueue;

	ArchiveShared m_archive;
};
