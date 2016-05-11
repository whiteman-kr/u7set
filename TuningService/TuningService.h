#pragma once

#include "../include/Service.h"
#include "../include/ServiceSettings.h"
#include "TuningDataSource.h"


class TuningServiceWorker : public ServiceWorker
{
private:
	TuningServiceSettings m_tuningSettings;
	TuningDataSources m_dataSources;

	bool readTuningDataSources(XmlReadHelper& xml);

	void clear();

public:
	TuningServiceWorker(const QString& serviceStrID,
						const QString& cfgServiceIP1,
						const QString& cfgServiceIP2);

	~TuningServiceWorker();

	TuningServiceWorker* createInstance() override;

	bool loadConfigurationFromFile(const QString& fileName);

	virtual void initialize() override;
	virtual void shutdown() override;
};

