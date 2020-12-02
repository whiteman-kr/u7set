#pragma once

#include "../lib/Service.h"
#include "../lib/SoftwareSettings.h"
#include "../lib/CfgServerLoader.h"
#include "TuningSource.h"
#include "TcpTuningServer.h"
#include "TuningSourceThread.h"
#include "TuningClientContext.h"

namespace Tuning
{
	class TcpTuningServerThread;


	class TuningServiceWorker : public ServiceWorker
	{
		Q_OBJECT

		static const QString SETTING_TUNING_SIM_IP;

	public:
		TuningServiceWorker(const SoftwareInfo& softwareInfo,
							const QString &serviceInstanceName,
							int &argc,
							char **argv,
							CircularLoggerShared logger,
							CircularLoggerShared tuningLog);
		~TuningServiceWorker();

		virtual ServiceWorker* createInstance() const override;
		virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const override;

		const TuningClientContext* getClientContext(QString clientID) const;
		const TuningClientContext* getClientContext(const std::string& clientID) const;

		const TuningSourceThread* getSourceThread(quint32 sourceIP) const;

		void getAllClientContexts(QVector<const TuningClientContext*>& clientContexts);

		bool singleLmControl() const;

		// called from TcpTuningServer thread!!!
		//
		NetworkError changeControlledTuningSource(const QString& tuningSourceEquipmentID,
													bool activateControl,
													QString* controlledTuningSource,
													bool* controlIsActive);

		bool clientIsConnected(const SoftwareInfo& softwareInfo, const QString& clientIP);
		bool clientIsDisconnected(const SoftwareInfo& softwareInfo, const QString& clientIP);
		bool setActiveClient(const SoftwareInfo& softwareInfo, const QString& clientIP);

		QString activeClientID() const;
		QString activeClientIP() const;

	signals:

	public slots:

	private:
		virtual void initCmdLineParser() override;
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
		bool readTuningDataSources(const QByteArray& fileData);

		void runTcpTuningServerThread();
		void stopTcpTuningServerThread();

		void runTuningSourceThreads();
		bool runTuningSourceThread(const QString& tuningSourceEquipmentID);		// if tuningSourceEquipmentID empty - run all sources workers
		void stopTuningSourceThreads();

		void runSourcesListenerThread();
		void stopSourcesListenerThread();

		void setSourceThreadInTuningClientContexts(TuningSourceThread* thread);
		void removeSourceThreadFromTuningClientContexts(TuningSourceThread* thread);

		bool isSimulationMode() const;

	private slots:
		void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	private:
		QString m_tuningSimIPStr;
		HostAddressPort m_tuningSimIP;

		CircularLoggerShared m_logger;
		CircularLoggerShared m_tuningLog;

		TuningServiceSettings m_cfgSettings;

		TuningSources m_tuningSources;

		CfgLoaderThread* m_cfgLoaderThread = nullptr;

		TcpTuningServerThread* m_tcpTuningServerThread = nullptr;

		mutable QMutex m_mainMutex;

		TuningSourceThreadMap m_sourceThreadMap;

		TuningSocketListenerThread* m_socketListenerThread = nullptr;

		TuningClientContextMap m_clientContextMap;

		SoftwareInfo m_activeClientInfo;
		QString m_activeClientIP;
	};
}
