#pragma once

#include "../ServiceLib/Service.h"
#include "../lib/SoftwareSettings.h"
#include "../OnlineLib/CfgServerLoader.h"
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

	public:
		TuningServiceWorker(const SoftwareInfo& softwareInfo,
							const QString &serviceInstanceName,
							int &argc,
							char **argv,
							CircularLoggerShared logger,
							E::ServiceRunMode runMode,
							CircularLoggerShared tuningLog);
		~TuningServiceWorker();

		virtual ServiceWorker* createInstance() const override;
		virtual void getServiceSpecificInfo(Network::ServiceInfo& servicesInfo) const override;

		const TuningClientContext* getClientContext(QString clientID) const;
		const TuningClientContext* getClientContext(const std::string& clientID) const;

		TuningSourceThread* getTuningSourceThread(quint32 sourceIP);

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
		bool readTuningDataSources(const QByteArray& fileData, const QString& profile);

		void runTcpTuningServerThread();
		void stopTcpTuningServerThread();

		void runTuningSourceThreads();
		bool runTuningSourceThread(bool runSingleSource,
								   const QString& tuningSourceEquipmentID);

		TuningSourceThreadShared createTuningSourceThread(const TuningSource& source);
		void stopTuningSourceThreads();

		void runSourcesListenerThreads();
		void stopSourcesListenerThreads();

		void setSourceThreadInTuningClientContexts(TuningSourceThread* thread);
		void removeSourceThreadFromTuningClientContexts(const QString& tuningSourceID);

		bool isSimulationMode() const;

		bool isSourceHandlerExistsForChannel(int channel) const;

	private slots:
		void onConfigurationReady(const QByteArray configurationXmlData,
								  const BuildFileInfoArray buildFileInfoArray,
								  SessionParams sessionParams,
								  std::shared_ptr<const SoftwareSettings> curSettingsProfile);

	private:
		CircularLoggerShared m_logger;
		CircularLoggerShared m_tuningLog;

		TuningServiceSettings m_settings;

		std::map<QString, TuningSourceThreadShared> m_sourceThreads;	// module EquipmentID => TuningSourceThreadShared
		std::map<quint32, TuningSourceThreadShared> m_ip2sourceThread;	// TuningSource LANs ipV4 => TuningSourceThreadShared

		TuningSources m_tuningSources;

		CfgLoaderThread* m_cfgLoaderThread = nullptr;

		TcpTuningServerThread* m_tcpTuningServerThread = nullptr;

		mutable QMutex m_mainMutex;

		std::vector<TuningSocketListenerThread*> m_socketListenerThreads;

		TuningClientContextMap m_clientContextMap;

		SoftwareInfo m_activeClientInfo;
		QString m_activeClientIP;
	};
}
