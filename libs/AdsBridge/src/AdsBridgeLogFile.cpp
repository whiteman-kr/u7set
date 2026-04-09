#include "AdsBridgeLogFile.h"

namespace AdsBridge
{
	MatsLogHandler LogFile::g_logHandler = nullptr;
	MatsLogLevel LogFile::g_logLevel = MATS_LOG_LEVEL_WARNING;

	void LogFile::writeAlert(std::string_view message)
	{
		return privateLog(MATS_LOG_LEVEL_ERROR, message.data());
	}

	void LogFile::writeError(std::string_view message)
	{
		return privateLog(MATS_LOG_LEVEL_ERROR, message.data());
	}

	void LogFile::writeWarning(std::string_view message)
	{
		return privateLog(MATS_LOG_LEVEL_WARNING, message.data());
	}

	void LogFile::writeMessage(std::string_view message)
	{
		return privateLog(MATS_LOG_LEVEL_DEBUG, message.data());
	}

	void LogFile::privateLog(MatsLogLevel level, const char* message)
	{
		if (g_logHandler != nullptr && level >= g_logLevel)
		{
			g_logHandler(level, message);
		}

		return;
	}
} // namespace AdsBridge