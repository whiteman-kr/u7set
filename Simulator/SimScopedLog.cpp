#include "SimScopedLog.h"
#include <QTime>

Q_LOGGING_CATEGORY(u7sim, "u7.sim")

namespace Sim
{

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
			writeText("Instance created.");
		}

		return;
	}

	ScopedLog::~ScopedLog()
	{
		if (m_scope.isEmpty() == false)
		{
			writeText("Instance destroyed.");
		}

		return;
	}

	bool ScopedLog::writeAlert(const QString& text)
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

	bool ScopedLog::writeError(const QString& text)
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

	bool ScopedLog::writeWarning(const QString& text)
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

	bool ScopedLog::writeMessage(const QString& text)
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

	bool ScopedLog::writeText(const QString& text)
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

	bool ScopedLog::writeDebug(const QString& text)
	{
		return writeText(text);
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
