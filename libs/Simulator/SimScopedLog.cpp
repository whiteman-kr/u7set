#include "SimScopedLog.h"


namespace Sim
{
	ScopedLog::ScopedLog(const ScopedLog& src) :
		m_log(src.m_log),
		m_scope(src.m_scope),
		m_debugMessagesEnabled(src.m_debugMessagesEnabled.load())
	{
	}

	ScopedLog::ScopedLog(ILogFile* log, bool allowDebugMessages, QString scope) :
		m_log(log),
		m_scope(scope),
		m_debugMessagesEnabled(allowDebugMessages)
	{
		if (m_scope.isEmpty() == false)
		{
			writeText("Instance created.");
		}

		return;
	}

	ScopedLog::ScopedLog(const ScopedLog& src, QString scope) :
		m_log(src.m_log),
		m_scope(scope),
		m_debugMessagesEnabled(src.m_debugMessagesEnabled.load())
	{
		if (m_scope.isEmpty() == false)
		{
			writeDebug("Instance created.");
		}

		return;
	}

	ScopedLog::~ScopedLog()
	{
		if (m_scope.isEmpty() == false)
		{
			writeDebug("Instance destroyed.");
		}

		return;
	}

	bool ScopedLog::writeAlert(QString text)
	{
		bool ok = true;

		if (m_log != nullptr)
		{
			QString logText = m_scope.isEmpty() ? text : m_scope + QStringLiteral(" ") + text;
			ok = m_log->writeAlert(logText);
		}

		return ok;
	}

	bool ScopedLog::writeError(QString text)
	{
		bool ok = true;

		if (m_log != nullptr)
		{
			QString logText = m_scope.isEmpty() ? text : m_scope + QStringLiteral(" ") + text;
			ok = m_log->writeError(logText);
		}

		return ok;
	}

	bool ScopedLog::writeWarning(QString text)
	{
		bool ok = true;

		if (m_log != nullptr)
		{
			QString logText = m_scope.isEmpty() ? text : m_scope + QStringLiteral(" ") + text;
			ok = m_log->writeWarning(logText);
		}

		return ok;
	}

	bool ScopedLog::writeMessage(QString text)
	{
		bool ok = true;

		if (m_log != nullptr)
		{
			QString logText = m_scope.isEmpty() ? text : m_scope + QStringLiteral(" ") + text;
			ok = m_log->writeMessage(logText);
		}

		return ok;
	}

	bool ScopedLog::writeText(QString text)
	{
		bool ok = true;

		if (m_log != nullptr)
		{
			QString logText = m_scope.isEmpty() ? text : m_scope + QStringLiteral(" ") + text;
			ok = m_log->writeText(logText);
		}

		return ok;
	}

	bool ScopedLog::writeDebug(QString text)
	{
		if (debugMessagesEnabled() == true)
		{
			return writeText(text);
		}

		return true;
	}

	const QString& ScopedLog::outputScope() const
	{
		return m_scope;
	}

	void ScopedLog::setOutputScope(QString value)
	{
		m_scope = value;
	}

	bool ScopedLog::debugMessagesEnabled() const
	{
		return m_debugMessagesEnabled.load(std::memory_order::relaxed);
	}

	void ScopedLog::setDebugMessagesEnabled(bool value)
	{
		m_debugMessagesEnabled.store(value, std::memory_order::relaxed);
	}

	ILogFile* ScopedLog::logInterface()
	{
		return m_log;
	}
}
