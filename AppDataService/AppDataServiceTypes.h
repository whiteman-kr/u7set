#pragma once

#include "../lib/Signal.h"
#include "../lib/OrderedHash.h"
#include "../lib/AppDataSource.h"



const int NO_INDEX = -1;
const int NO_AUTOARCHIVING_GROUP = -1;
const int NOT_INITIALIZED_AUTOARCHIVING_GROUP = -2;


struct SignalParseInfo
{
public:
	Address16 valueAddr;
	Address16 validityAddr;

	E::SignalType type = E::SignalType::Discrete;
	E::AnalogAppSignalFormat analogSignalFormat = E::AnalogAppSignalFormat::Float32;
	E::ByteOrder byteOrder = E::ByteOrder::BigEndian;

	int dataSize = 1;

	int index = NO_INDEX;		// index of signal in AppSignals and AddSignalStates

	QString appSignalID;

	void setSignalParams(int i, const Signal& s);
};


class SourceSignalsParseInfo : public QVector<SignalParseInfo>
{
public:
	SourceSignalsParseInfo(int autoArchivingGroupsCount);

	int getAutoArchivingGroup(qint64 currentSysTime);

private:
	static const int TIME_1S = 1000;

	int m_autoArchivingGroupsCount = 0;
	qint64 m_lastAutoArchivingTime = 0;
	int m_lastAutoArchivingGroup = NOT_INITIALIZED_AUTOARCHIVING_GROUP;
};


class SourceParseInfoMap : public QHash<quint32, SourceSignalsParseInfo*>
{
public:
	~SourceParseInfoMap();

	void clear();
};
