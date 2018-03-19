#pragma once

#include "../lib/DataSource.h"
#include "AppSignalStateEx.h"
#include "AppDataServiceTypes.h"
#include "AppDataProcessingThread.h"


class AppDataSource : public DataSource
{
public:
	AppDataSource();

	bool getState(Network::AppDataSourceState* protoState) const;
	bool setState(const Network::AppDataSourceState& protoState);

private:
	// app data parsing
	//
	SourceParseInfoMap m_sourceParseInfoMap;		// source ip => QVector<SignalParseInfo> map
	AppSignalStates* m_signalStates = nullptr;		// allocated and freed in AppDataService
	AppDataProcessingThreadsPool m_processingThreadsPool;
	AppSignalStatesQueue& m_signalStatesQueue;
};

typedef std::shared_ptr<AppDataSource> AppDataSourceShared;

typedef QHash<QString, AppDataSourceShared> AppDataSources;		// app data source EquipmentID => AppDataSourceShared

typedef QHash<quint32, AppDataSourceShared> AppDataSourcesIP;	// app data source IP => AppDataSourceShared
