#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <QtSql/QSqlRecord>

#include <ArchV3Lib/DbTypes.h>
#include <ArchV3Lib/Storage.h>

namespace ArchV3
{
	bool ArchFileInfo::fromQuery(const QSqlQuery& q)
	{
		if (!q.isValid())
		{
			Q_ASSERT(false);
			return false;
		}

		static constexpr int COL_ARCH_FILE_ID = 0;
		static constexpr int COL_SIGNAL_ID = 1;

		static constexpr int COL_HASH = 2;
		static constexpr int COL_BUCKET = 3;
		static constexpr int COL_SIGNAL_TYPE = 4;

		static constexpr int COL_FILE_NAME = 5;

		static constexpr int COL_CREATED_UTC = 6;
		static constexpr int COL_TIME_FROM_UTC = 7;
		static constexpr int COL_TIME_TO_UTC = 8;

		static constexpr int COL_RECORD_COUNT = 9;
		static constexpr int COL_FILE_SIZE = 10;

		static constexpr int COL_COMPLETED = 11;
		static constexpr int COL_COMPRESSED = 12;
		static constexpr int COL_DELETED = 13;

		static constexpr int COLUMNS_COUNT = 14;

		if (q.record().count() != COLUMNS_COUNT)
		{
			Q_ASSERT(false);
			return false;
		}

		auto b = q.value(COL_BUCKET).toUInt();

		if (b >= Storage::BUCKET_COUNT)
		{
			Q_ASSERT(false);
			return false;
		}

		E::SignalType st = static_cast<E::SignalType>(q.value(COL_SIGNAL_TYPE).toInt());

		if (st != E::SignalType::Analog && st != E::SignalType::Discrete)
		{
			Q_ASSERT(false);
			return false;
		}

		archFileID = q.value(COL_ARCH_FILE_ID).toLongLong();
		signalID = q.value(COL_SIGNAL_ID).toLongLong();

		hash = static_cast<Hash>(q.value(COL_HASH).toULongLong());

		bucket = static_cast<quint8>(b);
		signalType = st;

		fileName = q.value(COL_FILE_NAME).toString();

		createdUTC = q.value(COL_CREATED_UTC).toLongLong();
		timeFromUTC = q.value(COL_TIME_FROM_UTC).toLongLong();
		timeToUTC = q.value(COL_TIME_TO_UTC).toLongLong();

		recordCount = q.value(COL_RECORD_COUNT).toLongLong();
		fileSize = q.value(COL_FILE_SIZE).toLongLong();

		completed = q.value(COL_COMPLETED).toBool();
		compressed = q.value(COL_COMPRESSED).toBool();
		deleted = q.value(COL_DELETED).toBool();

		return true;
	}
} // namespace ArchV3