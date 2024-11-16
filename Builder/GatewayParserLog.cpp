#include "GatewayParserLog.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	// Class Gateway::ParserLog implementation
	//
	// ---------------------------------------------------------------------------------

	void ParserLog::logResult(int lineNo, LogMsgType msgType, const QString& msg)
	{
		log(lineNo, msgType, message(lineNo, msg));
	}

	void ParserLog::logError(int lineNo, const QString& errMsg)
	{
		log(lineNo, LogMsgType::Error, message(lineNo, errMsg));
	}

	void ParserLog::logError(const QString& errMsg)
	{
		log(0, LogMsgType::Error, errMsg);
	}

	void ParserLog::logWarning(int lineNo,
							   const QString& wrnMsg)
	{
		log(lineNo, LogMsgType::Warning, message(lineNo, wrnMsg));
	}

	void ParserLog::logWarning(const QString& wrnMsg)
	{
		log(0, LogMsgType::Warning, wrnMsg);
	}

	int ParserLog::errorCount() const
	{
		return m_errCount;
	}

	int ParserLog::warningCount() const
	{
		return m_wrnCount;
	}

	QString ParserLog::message(int lineNo, const QString& msg)
	{
		if (lineNo == 0)
		{
			return msg;
		}

		return QString("line %1, %2").arg(lineNo).arg(msg);
	}

	void ParserLog::log(int lineNo, LogMsgType msgType, const QString& msg)
	{
		push_back({lineNo, msgType, msg});

		switch(msgType)
		{
		case LogMsgType::Error:
			m_errCount++;
			break;

		case LogMsgType::Warning:
			m_wrnCount++;
			break;

		case LogMsgType::Message:
			break;

		case LogMsgType::Nothing:
		default:
			Q_ASSERT(false);
		}
	}
}
