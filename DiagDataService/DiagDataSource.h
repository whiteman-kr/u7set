#pragma once

#include<queue>

#include <DiagStateLib/SimpleDiagSignalState.h>

#include "../OnlineLib/OnlineDataSource.h"
#include "../OnlineLib/DataSource.h"
#include "../OnlineLib/CircularLogger.h"
#include "DynamicDiagSignalState.h"

class DiagDataSource : public OnlineDataSource<SimpleDiagSignalState>
{
public:
	DiagDataSource(const DataSource& dataSource);
	~DiagDataSource();

	virtual bool parseRupData(	const Times& time,
								bool isSimPacket,
								quint16 packetNo,
								const char* rupData,
								int rupDataSize,
								const QThread* thread) override;

	virtual bool invalidateAllSignals(const QThread* thread) override;
};
