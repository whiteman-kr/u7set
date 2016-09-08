#pragma once

#include "../lib/Service.h"
#include "../lib/ServiceSettings.h"
#include "../lib/CfgServerLoader.h"
#include "../AppDataService/AppSignalStateEx.h"
#include "../TuningService/TuningSocket.h"
#include "TuningDataSource.h"


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

		bool loadConfigurationFromFile(const QString& fileName);
		bool readTuningDataSources(XmlReadHelper& xml);

		void allocateSignalsAndStates();

		void runTuningSocket();
		void stopTuningSocket();

	private slots:
		void onTimer();
		void onConfigurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

	private:
		TuningServiceSettings m_tuningSettings;

		TuningDataSources m_dataSources;

		QHash<QString, QString> m_signal2Source;

		AppSignals m_appSignals;
		AppSignalStates m_appSignalStates;

		CfgLoaderThread* m_cfgLoaderThread = nullptr;

		Tuning::TuningSocketWorker* m_tuningSocket = nullptr;
		SimpleThread* m_tuningSocketThread = nullptr;

		QTimer m_timer;
	};
}
