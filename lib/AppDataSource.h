#pragma once

#include "../lib/DataSource.h"


class AppDataSource : public DataSource
{
public:
	AppDataSource();

	bool getState(Network::AppDataSourceState* protoState) const;
	bool setState(const Network::AppDataSourceState& protoState);
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


