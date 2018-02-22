#pragma once

#include "../lib/Service.h"
#include "../lib/ServiceSettings.h"
#include "../lib/CfgServerLoader.h"
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
		TuningServiceWorker(const SoftwareInfo& softwareInfo,
							const QString &serviceName,
							int &argc,
							char **argv,
							std::shared_ptr<CircularLogger> logger);
		~TuningServiceWorker();

		virtual ServiceWorker* createInstance() const override;
		virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const;

		const TuningClientContext* getClientContext(QString clientID) const;
		const TuningClientContext* getClientContext(const std::string& clientID) const;

		void getAllClientContexts(QVector<const TuningClientContext*>& clientContexts);

		bool singleLmControl() const;

		// called from TcpTuningServer thread!!!
		//
		NetworkError changeControlledTuningSource(const QString& tuningSourceEquipmentID,
													bool activateControl,
													QString* controlledTuningSource,
													bool* controlIsActive);

		QString equipmentID() { return m_equipmentID; }
		QString cfgServiceIP1() { return m_cfgServiceIP1.addressPortStr(); }
		QString cfgServiceIP2() { return m_cfgServiceIP2.addressPortStr(); }

		bool clientIsConnected(const SoftwareInfo& softwareInfo, const QString& clientIP);
		bool clientIsDisconnected(const SoftwareInfo& softwareInfo, const QString& clientIP);
		bool setActiveClient(const SoftwareInfo& softwareInfo, const QString& clientIP);

		QString activeClientID() const;
		QString activeClientIP() const;

	signals:

	public slots:

	private:
		static const char* const SETTING_EQUIPMENT_ID;
		static const char* const SETTING_CFG_SERVICE_IP1;
		static const char* const SETTING_CFG_SERVICE_IP2;

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
		bool readTuningDataSources(XmlReadHelper& xml);

		void runTcpTuningServerThread();
		void stopTcpTuningServerThread();

		void runTuningSourceWorkers();
		bool runTuningSourceWorker(const QString& tuningSourceEquipmentID);		// if tuningSourceEquipmentID empty - run all sources workers
		void stopTuningSourceWorkers();

		void runSourcesListenerThread();
		void stopSourcesListenerThread();

		void setWorkerInTuningClientContext(TuningSourceWorker* worker);
		void removeWorkerFromTuningClientContext(TuningSourceWorker* worker);

	private slots:
		void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	private:
		QString m_equipmentID;

		HostAddressPort m_cfgServiceIP1;
		HostAddressPort m_cfgServiceIP2;

		std::shared_ptr<CircularLogger> m_logger;

		TuningServiceSettings m_cfgSettings;

		TuningSources m_tuningSources;

		CfgLoaderThread* m_cfgLoaderThread = nullptr;

		TcpTuningServerThread* m_tcpTuningServerThread = nullptr;

		mutable QMutex m_mainMutex;

		TuningSourceWorkerThreadMap m_sourceWorkerThreadMap;

		TuningSocketListenerThread* m_socketListenerThread = nullptr;

		TuningClientContextMap m_clientContextMap;

		SoftwareInfo m_activeClientInfo;
		QString m_activeClientIP;
	};
}
