#pragma once

#include "../../UtilsLib/ILogFile.h"
#include <AdsConnectionLib/ILoggerStd.h>

namespace ClientLib
{
	class LoggerStdAdapter : public ILoggerStd
	{
	public:
		explicit LoggerStdAdapter(ILogFile& logFile) :
			m_logFile(logFile)
		{
			return;
		}

		virtual void writeAlert(std::string_view message) override { m_logFile.writeAlert(QString::fromStdString(std::string{message})); }
		virtual void writeError(std::string_view message) override { m_logFile.writeError(QString::fromStdString(std::string{message})); }
		virtual void writeWarning(std::string_view message) override
		{
			m_logFile.writeWarning(QString::fromStdString(std::string{message}));
		}
		virtual void writeMessage(std::string_view message) override
		{
			m_logFile.writeMessage(QString::fromStdString(std::string{message}));
		}

		ILogFile& logFile() const { return m_logFile; }

	private:
		ILogFile& m_logFile;
	};
} // namespace ClientLib