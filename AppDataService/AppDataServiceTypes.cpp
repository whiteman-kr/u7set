#include "AppDataServiceTypes.h"



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
