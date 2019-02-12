#pragma once

#include <QtGlobal>

#include "../../lib/AppSignalStateFlags.h"
#include "../../lib/Types.h"
#include "../../lib/Crc16.h"

static const QString ARCH_FILE_NAME_TEMPLATE("2[0-9][0-9][0-9]_[0-1][0-9]_[0-3][0-9]_[0-2][0-9]_[0-5][0-9].");

static const QString LONG_TERM_ARCHIVE_EXTENSION("lta");			// Long Term Archive
static const QString SHORT_TERM_ARCHIVE_EXTENSION("sta");			// Short Term Archive

#pragma pack(push, 1)

struct ArchFileRecord
{
	struct
	{
//			qint64 archID;
		qint64 localTime;
		qint64 systemTime;
		qint64 plantTime;

		AppSignalStateFlags flags;
		double value;
	} state;

	quint16 crc16;

	void calcCRC16() { crc16 = calcCrc16(&state, sizeof(state)); }

	bool isNotCorrupted() const;
	bool isNotCorrupted(E::TimeType timeType) const;

	qint64 getTime(E::TimeType timeType) const;

	void offsetTimes(qint64 dt);

	//

	static const qint64 TIME_MAX_VALUE;
	static const qint64 TIME_MIN_VALUE;
};

#pragma pack(pop)

