#include "SimScriptConnection.h"
#include "SimScriptSimulator.h"

namespace Sim
{

	ScriptConnection::ScriptConnection(const ScriptConnection& src) :
		m_connection(src.m_connection)
	{
	}

	ScriptConnection::ScriptConnection(std::shared_ptr<Connection> connection) :
		m_connection(connection)
	{
	}

	ScriptConnection& ScriptConnection::operator=(const ScriptConnection& src)
	{
		m_connection = src.m_connection;
		return *this;
	}

	bool ScriptConnection::isNull() const
	{
		return m_connection == nullptr;
	}

	QString ScriptConnection::connectionId() const
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("Get connectionId error"));
			return {};
		}

		return m_connection->connectionId();
	}

	bool ScriptConnection::enabled() const
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("Connection error"));
			return {};
		}

		return m_connection->enabled();
	}

	void ScriptConnection::setEnabled(bool value)
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("Connection error"));
			return;
		}

		m_connection->setEnabled(value);
		return;
	}

	bool ScriptConnection::timeout() const
	{
		if (isNull() == true)
		{
			ScriptSimulator::throwScriptException(this, tr("Connection error"));
			return {};
		}

		return m_connection->timeout();
	}


}
