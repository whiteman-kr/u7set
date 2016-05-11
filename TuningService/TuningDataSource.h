#pragma once

#include "../include/DataSource.h"


class TuningDataSource : public DataSource
{

public:
	TuningDataSource();
};


class TuningDataSources : public QHash<quint32, TuningDataSource*>
{
public:
	~TuningDataSources();

	void clear();
};
