#pragma once

namespace Gateway
{
	enum class LogMsgType
	{
		Nothing,
		Message,
		Warning,
		Error
	};

	struct LogRecord
	{
		int lineNo;
		LogMsgType msgType;
		QString msg;
	};

	enum class ParseResult
	{
		Ok,
		Error,
		CriticalError
	};

	class ParserLog : public std::vector<LogRecord>
	{
	public:
		void logResult(int lineNo, LogMsgType msgType, const QString& msg);

		void logError(int lineNo, const QString& errMsg);
		void logError(const QString& errMsg);

		void logWarning(int lineNo, const QString& wrnMsg);
		void logWarning(const QString& wrnMsg);

		int errorCount() const;
		int warningCount() const;

	private:
		QString message(int lineNo, const QString& msg);
		void log(int lineNo, LogMsgType msgType, const QString& msg);

	private:
		int m_errCount = 0;
		int m_wrnCount = 0;
	};
}
