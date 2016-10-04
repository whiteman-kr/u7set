#pragma once

#include "../lib/Signal.h"
#include "../lib/OrderedHash.h"
#include "../lib/AppDataSource.h"



const int	NO_INDEX = -1;


struct SignalParseInfo
{
	Address16 valueAddr;
	Address16 validityAddr;

	E::SignalType type = E::SignalType::Discrete;
	E::AnalogAppSignalFormat analogSignalFormat = E::AnalogAppSignalFormat::Float32;
	E::ByteOrder byteOrder = E::ByteOrder::BigEndian;


	int dataSize = 1;

	int index = NO_INDEX;		// index of signal in AppSignals and AddSignalStates

	void setSignalParams(int i, const Signal& s);
};


typedef QVector<SignalParseInfo> SourceSignalsParseInfo;


class SourceParseInfoMap : public QHash<quint32, SourceSignalsParseInfo*>
{
public:
	~SourceParseInfoMap();

	void clear();
};
