#pragma once

#include "../lib/Service.h"
#include "../lib/CfgServerLoader.h"
#include "../lib/ServiceSettings.h"
#include "../lib/Queue.h"

#include "TcpAppDataServer.h"
#include "Archive.h"
#include "TcpArchRequestsServer.h"
#include "ArchRequest.h"
#include "ArchWriterThread.h"

class ArchivingService : public ServiceWorker
{
	Q_OBJECT

public:
	ArchivingService(const SoftwareInfo& softwareInfo,
						   const QString &serviceName,
						   int &argc,
						   char **argv,
						   std::shared_ptr<CircularLogger> logger);
	~ArchivingService();

	virtual ServiceWorker* createInstance() const override;
	virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const override;

private:
	virtual void initCmdLineParser() override;
	virtual void loadSettings() override;

	virtual void initialize() override;
	virtual void shutdown() override;

	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	void startAllThreads();
	void stopAllThreads();

	void startArchive();
	void stopArchive();

	void startTcpAppDataServerThread();
	void stopTcpAppDataServerThread();

	void startTcpArchRequestsServerThread();
	void stopTcpArchiveRequestsServerThread();

	bool loadConfigurationXml(const QByteArray& fileData, ArchivingServiceSettings* settings);

	bool loadArchSignalsProto(const QByteArray& fileData);
	void deleteArchSignalsProto();

	void logFileLoadResult(bool loadOk, const QString& fileName);

private slots:
	void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

private:
	QString m_overwriteArchiveLocation;
	int m_minQueueSizeForFlushing = 0;
	QSettings m_settings;

	ArchivingServiceSettings m_serviceSettings;
	Builder:: BuildInfo m_buildInfo;
	Proto::ArchSignals* m_archSignalsProto = nullptr;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	Tcp::ServerThread* m_tcpAppDataServerThread = nullptr;
	Tcp::ServerThread* m_tcpArchRequestsServerThread = nullptr;

	Archive* m_archive = nullptr;

	static const char* const SETTING_ARCHIVE_LOCATION;
	static const char* const SETTING_MIN_QUEUE_SIZE_FOR_FLUSHING;
};
