#pragma once

#include <QtGlobal>

#include "../CommonLib/Types.h"
#include "../lib/AppSignal.h"
#include "../UtilsLib/Crc.h"

static const QString ARCH_FILE_NAME_TEMPLATE("2[0-9][0-9][0-9]_[0-1][0-9]_[0-3][0-9]_[0-2][0-9]_[0-5][0-9].[ls]ta");

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

		quint16 packetNo;

		AppSignalStateFlags flags;
		double value;
	} state;

	quint16 crc16;

	void calcCRC16() { crc16 = calcCrc16(&state, sizeof(state)); }

	bool isNotCorrupted() const;
	bool isNotCorrupted(E::TimeType timeType) const;

	qint64 getTime(E::TimeType timeType) const;

	void offsetTimes(qint64 dt);

	bool hasShortTermArchivingReasonOnly() const { return state.flags.hasShortTermArchivingReasonOnly(); }

	//

	static const qint64 TIME_MAX_VALUE;
	static const qint64 TIME_MIN_VALUE;

	static const int SIZE;
};

#pragma pack(pop)

