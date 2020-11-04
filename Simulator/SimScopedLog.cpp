#include "SimScopedLog.h"
#include <QTime>

Q_LOGGING_CATEGORY(u7sim, "u7.sim")

namespace Sim
{
	ScopedLog::ScopedLog(const ScopedLog& src) :
		m_log(src.m_log),
		m_scope(src.m_scope)
	{
	}

	ScopedLog::ScopedLog(ILogFile* log, QString scope) :
		m_log(log),
		m_scope(scope)
	{
		if (m_scope.isEmpty() == false)
		{
			writeText("Instance created.");
		}

		return;
	}

	ScopedLog::ScopedLog(ScopedLog src, QString scope) :
		m_log(src.m_log),
		m_scope(scope)
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
		// ScopedLog is used for tests, to prevent from rubishing script output writeDebug is disabled
		//return writeText(text);
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


}
