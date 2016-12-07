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
		TuningServiceWorker(const QString& serviceEquipmentID,
							const QString& cfgServiceIP1,
							const QString& cfgServiceIP2,
							const QString& buildPath);

		~TuningServiceWorker();

		virtual TuningServiceWorker* createInstance() override;

		const TuningClientContext* getClientContext(QString clientID) const;
		const TuningClientContext* getClientContext(const std::string& clientID) const;

	signals:

	public slots:

	private:
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
		TuningServiceSettings m_tuningSettings;
		TuningSources m_tuningSources;

		CfgLoaderThread* m_cfgLoaderThread = nullptr;

		TcpTuningServerThread* m_tcpTuningServerThread = nullptr;

		TuningSourceWorkerThreadMap m_sourceWorkerThreadMap;

		TuningSocketListenerThread* m_socketListenerThread = nullptr;

		QTimer m_timer;

		TuningClientContextMap m_clientContextMap;
	};
}
