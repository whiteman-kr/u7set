#pragma once

#include "../include/DataSource.h"
#include "../u7/Builder/TuningDataStorage.h"


class TuningDataSourceInfo
{
public:
	int channel = 0;
	DataSource::DataType dataType = DataSource::DataType::App;
	QString lmEquipmentID;								// unique ID
	QString lmCaption;
	QString lmAdapterID;
	bool lmDataEnable;
	HostAddressPort lmAddressPort;
	ulong lmDataID = 0;

	QVector<Signal> tuningSignals;
};


class TuningDataSource : public DataSource
{
private:
	TuningData* m_tuningData = nullptr;

	bool m_deleteTuningData = false;

public:
	TuningDataSource();
	~TuningDataSource();

	void setTuningData(TuningData* tuningData);

	virtual void writeAdditionalSectionsToXml(XmlWriteHelper& xml) override;
	virtual bool readAdditionalSectionsFromXml(XmlReadHelper& xml) override;

	void getTuningDataSourceInfo(TuningDataSourceInfo& info);
};


class TuningDataSources : public QHash<QString, TuningDataSource*>
{
public:
	~TuningDataSources();

	void clear();

	void getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info);
};
