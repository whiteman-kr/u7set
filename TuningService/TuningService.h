#pragma once

#include "../lib/Service.h"
#include "../lib/ServiceSettings.h"
#include "../lib/CfgServerLoader.h"
#include "../AppDataService/AppSignalStateEx.h"
#include "../TuningService/TuningSocket.h"
#include "TuningSource.h"
#include "TcpTuningServer.h"

namespace Tuning
{

	class TuningServiceWorker : public ServiceWorker
	{
		Q_OBJECT

	public:
		TuningServiceWorker(const QString& serviceEquipmentID,
							const QString& cfgServiceIP1,
							const QString& cfgServiceIP2,
							const QString& buildPath);

		~TuningServiceWorker();

		void clear();

		virtual TuningServiceWorker* createInstance() override;

	signals:

	public slots:

	private:
		virtual void initialize() override;
		virtual void shutdown() override;

		void runCfgLoaderThread();
		void stopCfgLoaderThread();
		void clearConfiguration();
		void applyNewConfiguration();

		bool readConfiguration(const QByteArray& cfgXmlData);
		bool loadConfigurationFromFile(const QString& fileName);
		bool readTuningDataSources(XmlReadHelper& xml);

		void allocateSignalsAndStates();

		void runTcpTuningServerThread();
		void stopTcpTuningServerThread();

		void runTuningSocket();
		void stopTuningSocket();


	private slots:
		void onTimer();
		void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	private:
		TuningServiceSettings m_tuningSettings;
		TuningSources m_tuningSources;

		CfgLoaderThread* m_cfgLoaderThread = nullptr;

		TcpTuningServerThread* m_tcpTuningServerThread = nullptr;

		Tuning::TuningSocketWorker* m_tuningSocket = nullptr;
		SimpleThread* m_tuningSocketThread = nullptr;

		QTimer m_timer;
	};
}
