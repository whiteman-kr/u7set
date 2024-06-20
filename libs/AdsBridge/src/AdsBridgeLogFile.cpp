#include "AdsBridgeLogFile.h"

namespace AdsBridge
{
	MatsLogHandler LogFile::g_logHandler = nullptr;
	MatsLogLevel LogFile::g_logLevel = LOG_LEVEL_WARNING;

	bool LogFile::writeAlert(const QString& text, [[maybe_unused]] const QString& tag)
	{
		return privateLog(LOG_LEVEL_ERROR, text.toStdString().c_str());
	}

	bool LogFile::writeError(const QString& text, [[maybe_unused]] const QString& tag)
	{
		return privateLog(LOG_LEVEL_ERROR, text.toStdString().c_str());
	}

	bool LogFile::writeWarning(const QString& text, [[maybe_unused]] const QString& tag)
	{
		return privateLog(LOG_LEVEL_WARNING, text.toStdString().c_str());
	}

	bool LogFile::writeMessage(const QString& text, [[maybe_unused]] const QString& tag)
	{
		return privateLog(LOG_LEVEL_DEBUG, text.toStdString().c_str());
	}

	bool LogFile::writeText(const QString& text, [[maybe_unused]] const QString& tag)
	{
		return privateLog(LOG_LEVEL_DEBUG, text.toStdString().c_str());
	}

	bool LogFile::privateLog(MatsLogLevel level, const char* message)
	{
		if (g_logHandler != nullptr && level >= g_logLevel)
		{
			g_logHandler(level, message);
		}

		return true;
	}
} // namespace AdsBridge