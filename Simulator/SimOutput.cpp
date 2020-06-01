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
			writeDebug("Instance created.");
		}

		return;
	}

	Output::~Output()
	{
		if (m_scope.isEmpty() == false)
		{
			writeDebug("Instance destroyed.");
		}

		return;
	}

	void Output::writeDebug(const QString& text)
	{
		if (m_scope.isEmpty() == true)
		{
			qCDebug(u7sim).noquote() << text;
		}
		else
		{
			qCDebug(u7sim).noquote() << m_scope  << text;
		}
	}

	void Output::writeDebug(const QString& text) const
	{
		return const_cast<Output*>(this)->writeDebug(text);
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

	void Output::writeMessage(const QString& text) const
	{
		return const_cast<Output*>(this)->writeMessage(text);
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

	void Output::writeWaning(const QString& text) const
	{
		return const_cast<Output*>(this)->writeWaning(text);
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

	void Output::writeError(const QString& text) const
	{
		return const_cast<Output*>(this)->writeError(text);
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
