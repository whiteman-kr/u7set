#include "SimConnections.h"

namespace Sim
{
	//
	// Sim::ConnectionPort
	//
	ConnectionPort::ConnectionPort(::ConnectionPortInfo port) :
		m_port(port)
	{
	}

	//
	// Sim::Connection
	//
	Connection::Connection(::ConnectionInfo buildConnection) :
		m_buildConnection(buildConnection)
	{
		for (const ::ConnectionPortInfo& bp : m_buildConnection.ports)
		{
			ConnectionPortPtr cp = std::make_shared<ConnectionPort>(bp);
			m_ports.push_back(cp);
		}

		return;
	}

	//
	// Sim::Connections
	//
	Connections::Connections()
	{
	}

	bool Connections::load(QString fileName, QString* errorMessage)
	{
		assert(errorMessage);

		bool ok = m_buildConnections.load(fileName, errorMessage);
		if (ok == false)
		{
			return false;
		}

		// m_connections
		//
		for (const ::ConnectionInfo& ci : m_buildConnections.connections)
		{
			ConnectionPtr c = std::make_shared<Sim::Connection>(ci);
			m_connections.push_back(c);

			// m_lmToConnection

		}

		return ok;
	}
}
