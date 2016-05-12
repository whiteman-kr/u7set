#pragma once

#include "../include/Service.h"
#include "../include/ServiceSettings.h"
#include "../AppDataService/AppSignalState.h"
#include "../TuningService/TuningSocket.h"
#include "TuningDataSource.h"


class TuningService;


class TuningServiceWorker : public ServiceWorker
{
	Q_OBJECT

private:
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

private slots:
	void onTimer();

public:
	TuningServiceWorker(const QString& serviceStrID,
						const QString& cfgServiceIP1,
						const QString& cfgServiceIP2,
						const QString& cfgFileName);

	void setTuningService(TuningService* tuningService) { m_tuningService = tuningService; }

	~TuningServiceWorker();

	TuningServiceWorker* createInstance() override;

	bool loadConfigurationFromFile(const QString& fileName);
	void getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info);

	virtual void initialize() override;
	virtual void shutdown() override;

signals:
	void tuningServiceReady();

public slots:
	void setSignalValue(QString appSignalID, double value);
};



class TuningService : public Service
{
	Q_OBJECT

private:
	TuningServiceWorker* m_tuningServiceWorker = nullptr;

	void setTuningServiceWorker(TuningServiceWorker* tuningServiceWorker) { m_tuningServiceWorker = tuningServiceWorker; }

public:
	TuningService(TuningServiceWorker* worker);

	void getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info);

	void setSignalState(QString appSignalID, double value);
	void getSignalState(QString appSignalID);

	friend class TuningServiceWorker;

signals:
	void tuningServiceReady();
	void signalStateReady(QString appSignalID, double value);

	// for internal use only
	//
	void signal_setSignalState(QString appSignalID, double value);
	void signal_getSignalState(QString appSignalID);
};
