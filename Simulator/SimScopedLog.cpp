#include "SimScopedLog.h"
#include <QTime>

Q_LOGGING_CATEGORY(u7sim, "u7.sim")

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
		QString logText = m_scope.isEmpty() ? text : m_scope + QStringLiteral(" ") + text;
		bool ok = true;

		if (m_log == nullptr)
		{
			qCCritical(u7sim).noquote() << logText;
		}
		else
		{
			ok = m_log->writeAlert(logText);
		}

		return ok;
	}

	bool ScopedLog::writeError(QString text)
	{
		QString logText = m_scope.isEmpty() ? text : m_scope + QStringLiteral(" ") + text;
		bool ok = true;

		if (m_log == nullptr)
		{
			qCCritical(u7sim).noquote() << logText;
		}
		else
		{
			ok = m_log->writeError(logText);
		}

		return ok;
	}

	bool ScopedLog::writeWarning(QString text)
	{
		QString logText = m_scope.isEmpty() ? text : m_scope + QStringLiteral(" ") + text;
		bool ok = true;

		if (m_log == nullptr)
		{
			qCWarning(u7sim).noquote() << logText;
		}
		else
		{
			ok = m_log->writeWarning(logText);
		}

		return ok;
	}

	bool ScopedLog::writeMessage(QString text)
	{
		QString logText = m_scope.isEmpty() ? text : m_scope + QStringLiteral(" ") + text;
		bool ok = true;

		if (m_log == nullptr)
		{
			qCInfo(u7sim).noquote() << logText;
		}
		else
		{
			ok = m_log->writeMessage(logText);
		}

		return ok;
	}

	bool ScopedLog::writeText(QString text)
	{
		QString logText = m_scope.isEmpty() ? text : m_scope + QStringLiteral(" ") + text;
		bool ok = true;

		if (m_log == nullptr)
		{
			qCDebug(u7sim).noquote() << logText;
		}
		else
		{
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
		return m_debugMessagesEnabled;
	}

	void ScopedLog::setDebugMessagesEnabled(bool value)
	{
		m_debugMessagesEnabled = value;
	}


}
