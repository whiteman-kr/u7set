#pragma once

#include<queue>

#include <DiagStateLib/SimpleDiagSignalState.h>

#include "../OnlineLib/OnlineDataSource.h"
#include "../OnlineLib/DataSource.h"
#include "../OnlineLib/CircularLogger.h"
#include <HardwareLib/DiagSignalType.h>
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
								int rupDataSize) override;

	virtual bool invalidateAllSignals() override;

	bool init(const std::map<Hash, Hardware::DiagSignalType>& diagSignalTypes,
			  const ::Network::AcquiredDiagSignalsAndObjects& diagSignalsAndObjects);
};
