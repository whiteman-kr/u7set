#pragma once

#include "map"
#include "../lib/Hash.h"
#include "../lib/ConnectionsInfo.h"


namespace Sim
{
	class Connection;
	class ConnectionPort;

	using ConnectionPtr = std::shared_ptr<Sim::Connection>;
	using ConnectionPortPtr = std::shared_ptr<Sim::ConnectionPort>;


	//
	// ConnectionData
	//
	class ConnectionData
	{
	public:
		//ConnectionData() = delete;
		ConnectionData();

	public:
		int sizeBytes() const;
		int sizeWords() const;

	private:
		ConnectionPortPtr m_fromPort;
		ConnectionPtr m_connection;

		QByteArray m_data;			// Raw data from LM memory
		qint64 m_sentTime;			// When packet was "sent". If 0 then buffer is not valid
	};


	//
	// ConnectionPort
	//
	class ConnectionPort
	{
	public:
		ConnectionPort(::ConnectionPortInfo port);

	private:
		::ConnectionPortInfo m_port;
	};


	//
	// Connection
	//
	class Connection
	{
	public:
		Connection(::ConnectionInfo buildConnection);

	public:
		bool sendData(int portNo, QByteArray* data);
		bool getData(int portNo, QByteArray* data);

	private:
		::ConnectionInfo m_buildConnection;
		std::vector<Sim::ConnectionPortPtr> m_ports;

		std::atomic_bool m_enable = true;

		// Data sent by port 1, protected with a mutex
		//
		QMutex m_dataMutexPort1;
		ConnectionData m_port1sentData;

		// Data sent by port 2, protected with a mutex
		//
		QMutex m_dataMutexPort2;
		ConnectionData m_port2sentData;
	};


	//
	// Connections
	//
	class Connections
	{
	public:
		Connections();

		bool load(QString fileName, QString* errorMessage);

	private:
		::ConnectionsInfo m_buildConnections;

		std::vector<ConnectionPtr> m_connections;
		std::multimap<Hash, ConnectionPtr> m_lmToConnection;		// LM to connections
		std::map<Hash, ConnectionPtr> m_portToConnection;			// PortID to connection
	};

}


