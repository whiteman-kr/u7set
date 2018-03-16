#pragma once

#include "../lib/DataSource.h"



class AppDataSource : public DataSource
{
public:
	AppDataSource();

	bool getState(Network::AppDataSourceState* protoState) const;
	bool setState(const Network::AppDataSourceState& protoState);
};

typedef std::shared_ptr<AppDataSource> AppDataSourceShared;


class AppDataSources : public HashedVector<QString, AppDataSourceShared>
{
public:
	~AppDataSources();

	void clear();
};


class AppDataSourcesIP : public HashedVector<quint32, AppDataSourceShared>
{
public:
};


