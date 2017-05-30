#pragma once

#include "../lib/Service.h"
#include "../lib/ServiceSettings.h"
#include "../AppDataService/AppSignalStateEx.h"
#include "TuningIPENSource.h"
#include "TuningIPENSocket.h"


namespace TuningIPEN
{

	class TuningIPENService;

	class TuningIPENServiceWorker : public ServiceWorker
	{
		Q_OBJECT

	private:
		TuningIPENService* m_tuningIPENService = nullptr;

		std::shared_ptr<CircularLogger> m_logger;

		TuningServiceSettings m_tuningSettings;
		TuningSources m_dataSources;

		QHash<QString, QString> m_signal2Source;

		AppSignals m_appSignals;
		AppSignalStates m_appSignalStates;

		TuningIPEN::TuningIPENSocketWorker* m_tuningSocket = nullptr;
		SimpleThread* m_tuningSocketThread = nullptr;

		QTimer m_timer;

		QString m_cfgFileName;
		QString m_buildPath;

		bool readTuningDataSources(XmlReadHelper& xml);

		void clear();

		void allocateSignalsAndStates();

		void runTuningSocket();
		void stopTuningSocket();

		void sendPeriodicReadRequests();
		void sendPeriodicFrameRequest(TuningIPEN::TuningSource* source);
		void testConnections();
		void emitTuningDataSourcesStates();

		virtual void initialize() override;
		virtual void shutdown() override;

		virtual void requestPreprocessing(SocketRequest& sr);
		virtual void replyPreprocessing(SocketReply& sr);

		virtual void initCmdLineParser();
		virtual void processCmdLineSettings();
		virtual void loadSettings();

	private slots:
		void onTimer();
		void onReplyReady();
		void onGetSignalState(QString appSignalID);

	public slots:
		void onSetSignalState(QString appSignalID, double value);

	public:
		TuningIPENServiceWorker(const QString& serviceName,
								int& argc,
								char** argv,
								const VersionInfo& versionInfo,
								std::shared_ptr<CircularLogger> logger);
		virtual ~TuningIPENServiceWorker();

		virtual ServiceWorker* createInstance() const override;
		virtual void getServiceSpecificInfo(Network::ServiceInfo& serviceInfo) const override;

		void setTuningService(TuningIPENService* tuningService) { m_tuningIPENService = tuningService; }

		bool loadConfigurationFromFile(const QString& fileName);
		void getTuningDataSourcesInfo(QVector<TuningSourceInfo>& info);

	signals:
		void tuningServiceReady();
		void signalStateReady(QString appSignalID, double currentValue, double lowLimit, double highLimit, bool valid);
		void tuningDataSourceStateUpdate(TuningSourceState state);
	};


	class TuningIPENService : public Service
	{
		Q_OBJECT

	private:
		TuningIPENServiceWorker* m_tuningServiceWorker = nullptr;

	public:
		TuningIPENService(TuningIPENServiceWorker* worker, std::shared_ptr<CircularLogger> logger);

		void setTuningServiceWorker(TuningIPENServiceWorker* tuningServiceWorker) { m_tuningServiceWorker = tuningServiceWorker; }

		void getTuningDataSourcesInfo(QVector<TuningSourceInfo>& info);

		void setSignalState(QString appSignalID, double value);
		void getSignalState(QString appSignalID);

		friend class TuningServiceWorker;

	signals:
		void tuningServiceReady();
		void signalStateReady(QString appSignalID, double currentValue, double lowLimit, double highLimit, bool valid);
		void tuningDataSourceStateUpdate(TuningSourceState state);

		void userRequest(FotipFrame fotipFrame);
		void replyWithNoZeroFlags(FotipFrame fotipFrame);

		// for internal use only
		//
		void signal_setSignalState(QString appSignalID, double value);
		void signal_getSignalState(QString appSignalID);
	};

}
