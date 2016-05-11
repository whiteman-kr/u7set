#pragma once

#include "../include/DataSource.h"
#include "../u7/Builder/TuningDataStorage.h"

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
};


class TuningDataSources : public QHash<quint32, TuningDataSource*>
{
public:
	~TuningDataSources();

	void clear();
};
