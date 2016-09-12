#pragma once

#include "../lib/DataSource.h"
#include "AppSignalStateEx.h"


class AppDataSource : public DataSource
{
public:
	AppDataSource();
};



class AppDataSources : public HashedVector<QString, AppDataSource*>
{
public:
	~AppDataSources();

	void clear();
};


class AppDataSourcesIP : public HashedVector<quint32, AppDataSource*>
{
public:
};


