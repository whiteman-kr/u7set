#pragma once

#include "../include/Service.h"
#include "../include/ServiceSettings.h"
#include "TuningDataSource.h"
#include "../AppDataService/AppSignalState.h"


class TuningServiceWorker : public ServiceWorker
{
private:
	QString m_cfgFileName;
	TuningServiceSettings m_tuningSettings;
	TuningDataSources m_dataSources;

	AppSignals m_appSignals;
	AppSignalStates m_appSignalStates;

	bool readTuningDataSources(XmlReadHelper& xml);

	void clear();

	void allocateSignalsAndStates();

public:
	TuningServiceWorker(const QString& serviceStrID,
						const QString& cfgServiceIP1,
						const QString& cfgServiceIP2,
						const QString& cfgFileName);

	~TuningServiceWorker();

	TuningServiceWorker* createInstance() override;

	bool loadConfigurationFromFile(const QString& fileName);
	void getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info);

	virtual void initialize() override;
	virtual void shutdown() override;

public slots:
	void setSignalValue(QString appSignalID, double value);
};

