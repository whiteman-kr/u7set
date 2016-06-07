#pragma once

#include "../lib/DataSource.h"
#include "AppSignalStateEx.h"


class AppDataSource : public DataSource
{
public:
	AppDataSource();
};



class AppDataSources : public HashedVector<quint32, AppDataSource*>
{
public:
	~AppDataSources();

	void clear();
};

