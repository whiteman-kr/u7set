#include "SimOutput.h"
#include <QTime>

Q_LOGGING_CATEGORY(u7sim, "u7.sim")


namespace Sim
{
	Output::Output(QString scope) :
		m_scope(scope)
	{
		if (m_scope.isEmpty() == false)
		{
			writeMessage("Instance created.");
		}

		return;
	}

	Output::~Output()
	{
		if (m_scope.isEmpty() == false)
		{
			writeMessage("Instance destroyed.");
		}

		return;
	}

	void Output::writeMessage(const QString& text)
	{
		if (m_scope.isEmpty() == true)
		{
			qCInfo(u7sim).noquote() << text;
		}
		else
		{
			qCInfo(u7sim).noquote() << m_scope  << text;
		}
		return;
	}

	void Output::writeWaning(const QString& text)
	{
		if (m_scope.isEmpty() == true)
		{
			qCWarning(u7sim).noquote() << text;
		}
		else
		{
			qCWarning(u7sim).noquote() << m_scope << text;
		}
		return;
	}

	void Output::writeError(const QString& text)
	{
		if (m_scope.isEmpty() == true)
		{
			qCCritical(u7sim).noquote() << text;
		}
		else
		{
			qCCritical(u7sim).noquote() << m_scope << text;
		}
		return;
	}

	const QString& Output::outputScope() const
	{
		return m_scope;
	}

	void Output::setOutputScope(QString value)
	{
		m_scope = value;
	}
}
