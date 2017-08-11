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
// SourceSignalsParseInfo class implementation
//
// -------------------------------------------------------------------------------

SourceSignalsParseInfo::SourceSignalsParseInfo(int autoArchivingGroupsCount) :
	m_autoArchivingGroupsCount(autoArchivingGroupsCount)
{
}

int SourceSignalsParseInfo::getAutoArchivingGroup(qint64 currentSysTime)
{
	if (m_lastAutoArchivingTime == 0)
	{
		m_lastAutoArchivingTime = (currentSysTime / TIME_1S) * TIME_1S;		// rounds time to seconds
		m_lastAutoArchivingGroup = 0;

		return NO_AUTOARCHIVING_GROUP;
	}

	if (abs(currentSysTime - m_lastAutoArchivingTime) < TIME_1S)
	{
		return NO_AUTOARCHIVING_GROUP;
	}

	m_lastAutoArchivingTime = (currentSysTime / TIME_1S) * TIME_1S;		// rounds time to seconds

	int retGroup = m_lastAutoArchivingGroup;

	m_lastAutoArchivingGroup++;

	if (m_lastAutoArchivingGroup >= m_autoArchivingGroupsCount)
	{
		m_lastAutoArchivingGroup = 0;
	}

	return retGroup;
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
