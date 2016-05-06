#pragma once

#include "../include/Signal.h"
#include "../include/OrderedHash.h"
#include "AppDataSource.h"


class AppSignals : public HashedVector<QString, Signal*>
{
public:
	~AppSignals();

	void clear();
};


class AppDataSources : public HashedVector<quint32, AppDataSource*>
{
public:
	~AppDataSources();

	void clear();
};


const int	NO_INDEX = -1;


struct SignalParseInfo
{
	Address16 valueAddr;
	Address16 validityAddr;

	E::SignalType type = E::SignalType::Discrete;
	E::DataFormat dataFormat = E::DataFormat::UnsignedInt;

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
