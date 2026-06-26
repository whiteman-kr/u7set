#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <ArchV3Lib/Core.h>

namespace ArchV3
{
	Core::Core(const QString& archDir, const QString& projectName, const AppSignals& appSignals, CircularLoggerShared logger) :
		LogWrapper(logger, "ArchV3::Core"),
		m_archDir(archDir),
		m_projectName(projectName),
		m_appSignals(appSignals),
		m_logger(logger)
	{
	}

	Core::~Core() 
	{
	}

	ArchWriterShared Core::getArchWriter(const QString& srcEquipmentID)
	{
		std::lock_guard<std::mutex> lock(m_archWritersMutex);

		auto it = m_archWriters.find(srcEquipmentID);

		if (it != m_archWriters.end())
		{
			return it->second;
		}

		auto archWriter = std::make_shared<ArchWriter>(
			m_archDir,
			m_projectName,
			srcEquipmentID,
			m_appSignals,
			m_logger
		);

		m_archWriters[srcEquipmentID] = archWriter;
		return archWriter;
	}
} // namespace ArchV3