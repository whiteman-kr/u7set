#include "AppDataServiceTypes.h"


// -------------------------------------------------------------------------------
//
// AppSignals class implementation
//
// -------------------------------------------------------------------------------


AppSignals::~AppSignals()
{
	clear();
}


void AppSignals::clear()
{
	for(Signal* signal : *this)
	{
		delete signal;
	}

	HashedVector<QString, Signal*>::clear();
}


// -------------------------------------------------------------------------------
//
// AppDataSources class implementation
//
// -------------------------------------------------------------------------------

AppDataSources::~AppDataSources()
{
	clear();
}


void AppDataSources::clear()
{
	for(AppDataSource* dataSource : *this)
	{
		delete dataSource;
	}

	HashedVector<quint32, AppDataSource*>::clear();
}


// -------------------------------------------------------------------------------
//
// SignalParseInfo class implementation
//
// -------------------------------------------------------------------------------

void SignalParseInfo::setSignalParams(int i, const Signal& s)
{
	index = i;

	valueAddr = s.regValueAddr();
	validityAddr = s.regValidityAddr();

	type = s.type();
	dataFormat = s.dataFormat();
	byteOrder = s.byteOrder();
	dataSize = s.dataSize();
}


// -------------------------------------------------------------------------------
//
// SourceParseInfoMap class implementation
//
// -------------------------------------------------------------------------------

SourceParseInfoMap::~SourceParseInfoMap()
{
	clear();
}


void SourceParseInfoMap::clear()
{
	for(SourceSignalsParseInfo* sourceSignalsParseInfo : *this)
	{
		delete sourceSignalsParseInfo;
	}

	QHash<quint32, SourceSignalsParseInfo*>::clear();
}
