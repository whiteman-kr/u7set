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
	struct ConnectionData
	{
		std::vector<char> m_data;				// Raw data from LM memory
		std::chrono::microseconds m_sentTime{0};	// When packet was "sent". If 0 then buffer is not valid

		int sizeBytes() const;
		int sizeWords() const;
	};


	//
	// ConnectionPort
	//
	class ConnectionPort
	{
	public:
		ConnectionPort(::ConnectionPortInfo portInfo) :
			m_portInfo(portInfo)
		{
		}

	public:
		const ::ConnectionPortInfo& portInfo() const
		{
			return m_portInfo;
		}

	private:
		::ConnectionPortInfo m_portInfo;
	};


	//
	// Connection
	//
	class Connection
	{
	public:
		Connection(::ConnectionInfo buildConnection);
		~Connection();

	public:
		const QString& connectionId() const;

		Sim::ConnectionPortPtr portForLm(const QString& lmEquipmnetId);

		bool sendData(int portNo,
					  std::vector<char>* data,
					  std::chrono::microseconds currentTime);

		bool receiveData(int portNo,
						 std::vector<char>* data,
						 std::chrono::microseconds currentTime,
						 std::chrono::microseconds timeout,
						 bool* timeoutHappend);

		QString type() const;
		const ::ConnectionInfo& connectionInfo() const;

		const std::vector<Sim::ConnectionPortPtr>& ports() const;

		bool enabled() const;
		void setEnabled(bool value);

		std::vector<char>* getPortReceiveBuffer(int portNo);
		std::vector<char>* getPortSendBuffer(int portNo);

	private:
		::ConnectionInfo m_buildConnection;
		std::vector<Sim::ConnectionPortPtr> m_ports;

		std::atomic_bool m_enable = true;

		// Data sent by port 1, protected with a mutex
		//
		QMutex m_dataMutexPort1;
		ConnectionData m_port1sentData;

		std::vector<char> m_port1receiveBuffer;		// Receive buffer for port 1, accessed only by DeviceEmulator, in single thread
		std::vector<char> m_port1sendBuffer;		// Send buffer for port 1, accessed only by DeviceEmulator, in single thread

		// Data sent by port 2, protected with a mutex
		//
		QMutex m_dataMutexPort2;
		ConnectionData m_port2sentData;

		std::vector<char> m_port2receiveBuffer;		// Receive buffer for port 2, accessed only by DeviceEmulator, in single thread
		std::vector<char> m_port2sendBuffer;		// Send buffer for port 2, accessed only by DeviceEmulator, in single thread
	};


	//
	// Connections
	//
	class Connections : public QObject
	{
		Q_OBJECT

	public:
		Connections() = default;
		virtual ~Connections() = default;

	public:
		void clear();
		bool load(QString fileName, QString* errorMessage);

		ConnectionPtr connection(QString connectionId) const;
		std::vector<ConnectionPtr> connections() const;
		std::vector<ConnectionPtr> lmConnections(const QString& lmEquipmentId) const;

		void enableConnection(QString connectionId, bool enable);
		void disableConnection(QString connectionId, bool disable);

	signals:
		void connectionStateChanged(QString connectionId, bool state);

	private:
		::ConnectionsInfo m_buildConnections;

		std::vector<ConnectionPtr> m_connections;
		std::map<Hash, ConnectionPtr> m_connectionMap;				// ConnectionID to connection
		std::multimap<Hash, ConnectionPtr> m_lmToConnection;		// LM to connections
		std::map<Hash, ConnectionPtr> m_portToConnection;			// PortID to connection
		std::map<Hash, ConnectionPortPtr> m_portMap;				// PortID to connection port
	};

}


