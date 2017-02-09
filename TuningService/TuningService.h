#pragma once

#include "../lib/Service.h"
#include "../lib/ServiceSettings.h"
#include "../lib/CfgLoaderWithLog.h"
#include "../AppDataService/AppSignalStateEx.h"
#include "TuningSource.h"
#include "TcpTuningServer.h"
#include "TuningSourceWorker.h"
#include "TuningClientContext.h"

namespace Tuning
{
	class TcpTuningServerThread;


	class TuningServiceWorker : public ServiceWorker
	{
		Q_OBJECT

	public:
		TuningServiceWorker(const QString &serviceName, int &argc, char **argv, const VersionInfo &versionInfo);
		~TuningServiceWorker();

		virtual ServiceWorker* createInstance() const override;
		virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const;

		const TuningClientContext* getClientContext(QString clientID) const;
		const TuningClientContext* getClientContext(const std::string& clientID) const;

		void getAllClientContexts(QVector<const TuningClientContext*>& clientContexts);

	signals:

	public slots:

	private:
		virtual void initCmdLineParser() override;
		virtual void processCmdLineSettings() override;
		virtual void loadSettings() override;

		void clear();

		virtual void initialize() override;
		virtual void shutdown() override;

		void runCfgLoaderThread();
		void stopCfgLoaderThread();
		void clearConfiguration();
		void applyNewConfiguration();

		void buildServiceMaps();
		void clearServiceMaps();

		bool readConfiguration(const QByteArray& cfgXmlData);
		bool loadConfigurationFromFile(const QString& fileName);
		bool readTuningDataSources(XmlReadHelper& xml);

		void allocateSignalsAndStates();

		void runTcpTuningServerThread();
		void stopTcpTuningServerThread();

		void runTuningSourceWorkers();
		void stopTuningSourceWorkers();

		void setWorkerInTuningClientContext(const QString& sourceID, TuningSourceWorker* worker);

	private slots:
		void onTimer();
		void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	private:
		QString m_equipmentID;
		QString m_buildPath;
		QString m_cfgServiceIP1Str;
		QString m_cfgServiceIP2Str;

		HostAddressPort m_cfgServiceIP1;
		HostAddressPort m_cfgServiceIP2;

		TuningServiceSettings m_cfgSettings;

		TuningSources m_tuningSources;

		CfgLoaderThread* m_cfgLoaderThread = nullptr;

		TcpTuningServerThread* m_tcpTuningServerThread = nullptr;

		TuningSourceWorkerThreadMap m_sourceWorkerThreadMap;

		TuningSocketListenerThread* m_socketListenerThread = nullptr;

		QTimer m_timer;

		TuningClientContextMap m_clientContextMap;
	};
}
