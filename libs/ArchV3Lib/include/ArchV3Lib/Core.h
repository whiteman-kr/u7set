#pragma once

#include <unordered_map>

#include "../../AppSignalLib/AppSignal.h"
#include "../../OnlineLib/CircularLogger.h"

#include "Writer.h"
#include "Db.h"

namespace ArchV3
{
	class Core : public LogWrapper
	{
	public:
		Core(const QString& archDir, const QString& projectName, const AppSignals& appSignals, CircularLoggerShared logger);
		~Core();

		ArchWriterShared getArchWriter(const QString& srcEquipmentID);

	private:
		QString m_archDir;
		QString m_projectName;
		const AppSignals& m_appSignals;
		CircularLoggerShared m_logger;

		std::mutex m_archWritersMutex;
		std::unordered_map<QString, ArchWriterShared> m_archWriters;
	};
} // namespace ArchV3