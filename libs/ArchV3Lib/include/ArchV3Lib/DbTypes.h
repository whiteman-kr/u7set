#pragma once

#include <QtSql/QSqlQuery>

namespace ArchV3
{
	struct SignalRegisterInfo				// corresponded DB type: signal_register_info
	{
		QString appSignalID;
		Hash hash;
		E::SignalType signalType;
		quint8 bucket;
	};

	struct ArchFileInfo						// corresponded DB type: arch_file_info
	{
		qint64 archFileID = 0;
		qint64 signalID = 0;

		Hash hash = 0;
		quint8 bucket = 0;
		E::SignalType signalType = E::SignalType::Analog;

		QString fileName;

		qint64 createdUTC = 0;
		qint64 timeFromUTC = 0;
		qint64 timeToUTC = 0;

		qint64 recordCount = 0;
		qint64 fileSize = 0;

		bool completed = false;
		bool compressed = false;
		bool deleted = false;

		bool fromQuery(const QSqlQuery& q);

		bool operator==(const ArchFileInfo& other) const;
	};

} // namespace ArchV3