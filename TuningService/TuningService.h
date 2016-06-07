#pragma once

#include "../lib/Service.h"
#include "../lib/ServiceSettings.h"
#include "../AppDataService/AppSignalStateEx.h"
#include "../TuningService/TuningSocket.h"
#include "TuningDataSource.h"

namespace Tuning
{

	class TuningService;


	class TuningServiceWorker : public ServiceWorker
	{
		Q_OBJECT

	protected:
		QString m_cfgFileName;
		TuningService* m_tuningService = nullptr;

		TuningServiceSettings m_tuningSettings;
		TuningDataSources m_dataSources;

		QHash<QString, QString> m_signal2Source;

		AppSignals m_appSignals;
		AppSignalStates m_appSignalStates;

		Tuning::TuningSocketWorker* m_tuningSocket = nullptr;
		SimpleThread* m_tuningSocketThread = nullptr;

		QTimer m_timer;

		bool readTuningDataSources(XmlReadHelper& xml);

		void clear();

		void allocateSignalsAndStates();

		void runTuningSocket();
		void stopTuningSocket();

		void sendPeriodicReadRequests();
		void sendPeriodicFrameRequest(TuningDataSource* source);
		void testConnections();
		void emitTuningDataSourcesStates();

		virtual void initialize() override;
		virtual void shutdown() override;

	private slots:
		void onTimer();
		void onReplyReady();
		void onGetSignalState(QString appSignalID);

	public slots:
		void onSetSignalState(QString appSignalID, double value);

	public:
		TuningServiceWorker(const QString& serviceStrID,
							const QString& cfgServiceIP1,
							const QString& cfgServiceIP2,
							const QString& cfgFileName);

		void setTuningService(TuningService* tuningService) { m_tuningService = tuningService; }

		~TuningServiceWorker();

		virtual TuningServiceWorker* createInstance() override;

		bool loadConfigurationFromFile(const QString& fileName);
		void getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info);

		virtual void requestPreprocessing(Tuning::SocketRequest& sr);
		virtual void replyPreprocessing(Tuning::SocketReply& sr);

	signals:
		void tuningServiceReady();
		void signalStateReady(QString appSignalID, double currentValue, double lowLimit, double highLimit, bool valid);
		void tuningDataSourceStateUpdate(TuningDataSourceState state);
	};



	class TuningService : public Service
	{
		Q_OBJECT

	private:
		TuningServiceWorker* m_tuningServiceWorker = nullptr;

	public:
		TuningService(TuningServiceWorker* worker);

		void setTuningServiceWorker(TuningServiceWorker* tuningServiceWorker) { m_tuningServiceWorker = tuningServiceWorker; }

		void getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info);

		void setSignalState(QString appSignalID, double value);
		void getSignalState(QString appSignalID);

		friend class TuningServiceWorker;

	signals:
		void tuningServiceReady();
		void signalStateReady(QString appSignalID, double currentValue, double lowLimit, double highLimit, bool valid);
		void tuningDataSourceStateUpdate(TuningDataSourceState state);

		void userRequest(FotipFrame fotipFrame);
		void replyWithNoZeroFlags(FotipFrame fotipFrame);

		// for internal use only
		//
		void signal_setSignalState(QString appSignalID, double value);
		void signal_getSignalState(QString appSignalID);
	};

}
