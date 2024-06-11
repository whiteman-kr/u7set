#pragma once
#include "../../UtilsLib/ILogFile.h"
#include <AdsBridge/Common.h>

namespace AdsBridge
{
	class LogFile : public ILogFile
	{
	public:
		virtual bool writeAlert(const QString& text, const QString& tag = {}) override;
		virtual bool writeError(const QString& text, const QString& tag = {}) override;
		virtual bool writeWarning(const QString& text, const QString& tag = {}) override;
		virtual bool writeMessage(const QString& text, const QString& tag = {}) override;
		virtual bool writeText(const QString& text, const QString& tag = {}) override;

	private:
		bool privateLog(MatsLogLevel level, const char* message);

	public:
		static MatsLogHandler g_logHandler;
		static MatsLogLevel g_logLevel;
	};
} // namespace AdsBridge