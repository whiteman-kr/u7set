#include "ArchFileRecord.h"

// -----------------------------------------------------------------------------------------------------------------------
//
// ArchFileRecord struct implementation
//
// -----------------------------------------------------------------------------------------------------------------------

const int ArchFileRecord::SIZE = sizeof(ArchFileRecord);

bool ArchFileRecord::isNotCorrupted() const
{
	return	calcCrc16(this, sizeof(ArchFileRecord)) == 0;
}

bool ArchFileRecord::isNotCorrupted(E::TimeType timeType) const
{
	qint64 time = getTime(timeType);

	return	calcCrc16(this, sizeof(ArchFileRecord)) == 0 &&
			(time >= TIME_MIN_VALUE && time < TIME_MAX_VALUE);
}

qint64 ArchFileRecord::getTime(E::TimeType timeType) const
{
	switch(timeType)
	{
	case E::TimeType::Local:
		return state.localTime;

	case E::TimeType::System:
		return state.systemTime;

	case E::TimeType::Plant:
		return state.plantTime;

	case E::TimeType::ArchiveId:
		assert(false);				// not implemented now
		return 0;
	}

	assert(false);					// unknown time type
	return 0;
}

void ArchFileRecord::offsetTimes(qint64 dt)
{
	state.localTime += dt;
	state.systemTime += dt;
	state.plantTime += dt;
}

