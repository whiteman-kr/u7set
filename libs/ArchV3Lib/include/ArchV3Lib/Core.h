#pragma once

#include <unordered_map>

#include <ArchSignal.pb.h>

#include "../../AppSignalLib/AppSignal.h"
#include "../../OnlineLib/CircularLogger.h"

#include "Postgres.h"
#include "Db.h"
#include "ArchSignal.h"
#include "Writer.h"
#include "Storage.h"

namespace ArchV3
{
	class Core : public LogWrapper
	{
	public:
		Core(const QString& archDir, 
			const QByteArray& archInfoV3Data, 
			const DbConnectionInfo& dbConnInfo, 
			CircularLoggerShared logger);
		~Core();

		bool init();

		bool isWorkable() const;

		ArchWriterShared getArchWriter(const QString& srcEquipmentID);

	private:
		QString m_archDir;
		QString m_projectName;
		std::unique_ptr<Proto::ArchInfoV3> m_archInfoV3;
		const DbConnectionInfo m_dbConnInfo;
		CircularLoggerShared m_logger;

		std::atomic_bool m_workable{false};

		std::unordered_map<QString, std::vector<ArchSignal>> m_clientSignals;

		std::mutex m_archWritersMutex;
		std::unordered_map<QString, ArchWriterShared> m_archWriters;
	};
} // namespace ArchV3