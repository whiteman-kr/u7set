#include "AppDataServiceTypes.h"



// -------------------------------------------------------------------------------
//
// SignalParseInfo class implementation
//
// -------------------------------------------------------------------------------

void SignalParseInfo::setSignalParams(int i, const Signal& s)
{
	appSignalID = s.appSignalID();

	index = i;

	valueAddr = s.regValueAddr();
	validityAddr = s.regValidityAddr();

	type = s.signalType();
	analogSignalFormat = s.analogSignalFormat();
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
