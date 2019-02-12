#include "ArchFileRecord.h"

// -----------------------------------------------------------------------------------------------------------------------
//
// ArchFileRecord struct implementation
//
// -----------------------------------------------------------------------------------------------------------------------

bool ArchFileRecord::isValid() const
{
	return calcCrc16(this, sizeof(ArchFileRecord)) == 0;
}

qint64 ArchFileRecord::getTime(E::TimeType timeType)
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

