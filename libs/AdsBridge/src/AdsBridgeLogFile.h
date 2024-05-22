#pragma once
#include "../../UtilsLib/ILogFile.h"
#include <AdsBridge/Common.h>

namespace AdsBridge
{
	extern MatsLogHandler g_logHandler;
	extern MatsLogLevel g_logLevel;

	class LogFile : public ILogFile
	{
	public:
		virtual bool writeAlert(const QString& text, const QString& tag = {}) override;
		virtual bool writeError(const QString& text, const QString& tag = {}) override;
		virtual bool writeWarning(const QString& text, const QString& tag = {}) override;
		virtual bool writeMessage(const QString& text, const QString& tag = {}) override;
		virtual bool writeText(const QString& text, const QString& tag = {}) override;
	};
} // namespace AdsBridge