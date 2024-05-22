#include "AdsBridgeLogFile.h"

namespace AdsBridge
{
	MatsLogHandler g_logHandler = nullptr;
	MatsLogLevel g_logLevel = LOG_LEVEL_WARNING;

	static void private_log(MatsLogLevel level, const char* message)
	{
		if (g_logHandler != nullptr && level >= g_logLevel)
		{
			g_logHandler(level, message);
		}

		return;
	}

	bool LogFile::writeAlert(const QString& text, [[maybe_unused]] const QString& tag)
	{
		private_log(LOG_LEVEL_ERROR, text.toStdString().c_str());
		return true;
	}

	bool LogFile::writeError(const QString& text, [[maybe_unused]] const QString& tag)
	{
		private_log(LOG_LEVEL_ERROR, text.toStdString().c_str());
		return false;
	}

	bool LogFile::writeWarning(const QString& text, [[maybe_unused]] const QString& tag)
	{
		private_log(LOG_LEVEL_WARNING, text.toStdString().c_str());
		return false;
	}

	bool LogFile::writeMessage(const QString& text, [[maybe_unused]] const QString& tag)
	{
		private_log(LOG_LEVEL_DEBUG, text.toStdString().c_str());
		return false;
	}

	bool LogFile::writeText(const QString& text, [[maybe_unused]] const QString& tag)
	{
		private_log(LOG_LEVEL_DEBUG, text.toStdString().c_str());
		return false;
	}
} // namespace AdsBridge