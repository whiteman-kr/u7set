#include "SimException.h"

namespace Sim
{
	void SimException::raise(QString message, QString where /*= QString()*/, QString scope /*= QString()*/)
	{
		SimException e;

		e.m_message = message;
		e.m_where = where;
		e.m_scope = scope;

		e.raise();
	}

	void SimException::raise() const
	{
		throw *this;
	}

	SimException* SimException::clone() const
	{
		SimException* result = new SimException(*this);
		return result;
	}

	const QString& SimException::message() const
	{
		return m_message;
	}

	const QString& SimException::where() const
	{
		return m_where;
	}

	const QString& SimException::scope() const
	{
		return m_scope;
	}
}
