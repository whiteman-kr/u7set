#pragma once

#include "../lib/Service.h"
#include "../lib/ServiceSettings.h"
#include "../AppDataService/AppSignalStateEx.h"
#include "../TuningService/TuningDataSource.h"
#include "TuningIPENSocket.h"


namespace TuningIPEN
{

	class TuningIPENService;


	class TuningIPENServiceWorker : public ServiceWorker
	{
		Q_OBJECT

	private:
		TuningIPENService* m_tuningIPENService = nullptr;

		TuningServiceSettings m_tuningSettings;
		Tuning::TuningDataSources m_dataSources;

		QHash<QString, QString> m_signal2Source;

		AppSignals m_appSignals;
		AppSignalStates m_appSignalStates;

		TuningIPEN::TuningIPENSocketWorker* m_tuningSocket = nullptr;
		SimpleThread* m_tuningSocketThread = nullptr;

		QTimer m_timer;

		bool readTuningDataSources(XmlReadHelper& xml);

		void clear();

		void allocateSignalsAndStates();

		void runTuningSocket();
		void stopTuningSocket();

		void sendPeriodicReadRequests();
		void sendPeriodicFrameRequest(Tuning::TuningDataSource* source);
		void testConnections();
		void emitTuningDataSourcesStates();

		virtual void initialize() override;
		virtual void shutdown() override;

		virtual void requestPreprocessing(Tuning::SocketRequest& sr);
		virtual void replyPreprocessing(Tuning::SocketReply& sr);

	private slots:
		void onTimer();
		void onReplyReady();
		void onGetSignalState(QString appSignalID);

	public slots:
		void onSetSignalState(QString appSignalID, double value);

	public:
		TuningIPENServiceWorker(const QString& serviceEquipmentID,
							const QString& buildPath);

		void setTuningService(TuningIPENService* tuningService) { m_tuningIPENService = tuningService; }

		~TuningIPENServiceWorker();

		virtual TuningIPENServiceWorker* createInstance() override;

		bool loadConfigurationFromFile(const QString& fileName);
		void getTuningDataSourcesInfo(QVector<Tuning::TuningDataSourceInfo>& info);

	signals:
		void tuningServiceReady();
		void signalStateReady(QString appSignalID, double currentValue, double lowLimit, double highLimit, bool valid);
		void tuningDataSourceStateUpdate(Tuning::TuningDataSourceState state);
	};


	class TuningIPENService : public Service
	{
		Q_OBJECT

	private:
		TuningIPENServiceWorker* m_tuningServiceWorker = nullptr;

	public:
		TuningIPENService(TuningIPENServiceWorker* worker);

		void setTuningServiceWorker(TuningIPENServiceWorker* tuningServiceWorker) { m_tuningServiceWorker = tuningServiceWorker; }

		void getTuningDataSourcesInfo(QVector<Tuning::TuningDataSourceInfo>& info);

		void setSignalState(QString appSignalID, double value);
		void getSignalState(QString appSignalID);

		friend class TuningServiceWorker;

	signals:
		void tuningServiceReady();
		void signalStateReady(QString appSignalID, double currentValue, double lowLimit, double highLimit, bool valid);
		void tuningDataSourceStateUpdate(Tuning::TuningDataSourceState state);

		void userRequest(FotipFrame fotipFrame);
		void replyWithNoZeroFlags(FotipFrame fotipFrame);

		// for internal use only
		//
		void signal_setSignalState(QString appSignalID, double value);
		void signal_getSignalState(QString appSignalID);
	};

}
