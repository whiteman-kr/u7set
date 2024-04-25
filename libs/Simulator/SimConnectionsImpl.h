#pragma once

#include <HardwareLib/Connection.h>
#include <HardwareLib/ConnectionsInfo.h>

#include <Simulator/SimRam.h>

namespace Sim
{
	class ConnectionImpl;
	class ConnectionPort;

	using ConnectionImplPtr = std::shared_ptr<Sim::ConnectionImpl>;
	using ConnectionPortPtr = std::shared_ptr<Sim::ConnectionPort>;


	//
	// ConnectionData
	//
	struct ConnectionData
	{
		std::vector<char> m_data;					// Raw data from LM memory
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
		ConnectionPort(const ::ConnectionPortInfo& portInfo) :
			m_portInfo(portInfo)
		{
			m_lmIdHash = calcHash(m_portInfo.lmID);
		}

	public:
		const ::ConnectionPortInfo& portInfo() const
		{
			return m_portInfo;
		}

		Hash lmIdHash() const
		{
			return m_lmIdHash;
		}

		// writeValidityBit was moved here as optimisation, this is very convenient place 
		// to store memory area for this bit.
		//
		bool writeValidityBit(Ram& ram, quint16 value)
		{
			if (m_receiveValidityBitMemoryArea == Ram::InvalidHandle)
			{
				m_receiveValidityBitMemoryArea = ram.memoryAreaHandle(E::LogicModuleRamAccess::Read,
																	  m_portInfo.rxValiditySignalAbsAddr.offset());

				if (m_receiveValidityBitMemoryArea == Ram::InvalidHandle)
				{
					Q_ASSERT(false);
					return false;
				}
			}

			auto memoryArea = ram.memoryArea(m_receiveValidityBitMemoryArea);
			if (memoryArea == nullptr)
			{
				Q_ASSERT(false);
				return false;
			}

			return memoryArea->writeBit(m_portInfo.rxValiditySignalAbsAddr.offset(),
										static_cast<quint16>(m_portInfo.rxValiditySignalAbsAddr.bit()),
										value,
										E::ByteOrder::BigEndian);
		}

	private:
		::ConnectionPortInfo m_portInfo;
		Hash m_lmIdHash = UNDEFINED_HASH;

		Ram::Handle m_receiveValidityBitMemoryArea = Ram::InvalidHandle;
	};


	//
	// Connection
	//
	class ConnectionImpl
	{
	public:
		ConnectionImpl(const ::ConnectionInfo& buildConnection);

	public:
		const QString& connectionId() const;

		const Sim::ConnectionPort* portForLmRawPtr(const QString& lmEquipmnetId) const;
		const Sim::ConnectionPort* portForLmRawPtr(Hash logicModuleIdHash) const;
		Sim::ConnectionPort* portForLmRawPtr(Hash logicModuleIdHash);

		bool sendData(int portNo,
					  std::vector<char>* data,
					  std::chrono::microseconds currentTime);

		bool receiveData(int portNo,
						 std::vector<char>* data,
						 std::chrono::microseconds currentTime,
						 std::chrono::microseconds timeout,
						 bool* timeoutHappend);

		QString typeStr() const;
		Hardware::Connection::Type type() const;
		const ::ConnectionInfo& connectionInfo() const;

		std::vector<Sim::ConnectionPort> ports() const;

		bool enabled() const;
		void setEnabled(bool value);

		bool timeout() const;

		std::vector<char>* getPortReceiveBuffer(int portNo);
		std::vector<char>* getPortSendBuffer(int portNo);

	private:
		::ConnectionInfo m_buildConnection;

		std::optional<Sim::ConnectionPort> m_port1;
		std::optional<Sim::ConnectionPort> m_port2;

		std::atomic<bool> m_enable = true;
		std::atomic<bool> m_timeout = false;

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
	class ConnectionsImpl : public QObject
	{
		Q_OBJECT

	public:
		ConnectionsImpl(QObject* parent = nullptr);
		virtual ~ConnectionsImpl() = default;

	public:
		void clear();
		bool load(QString fileName, QString* errorMessage);

		ConnectionImplPtr connection(QString connectionId) const;
		std::vector<ConnectionImplPtr> connections() const;
		std::vector<ConnectionImplPtr> lmConnections(const QString& lmEquipmentId) const;

		void enableConnection(QString connectionId, bool enable);
		void disableConnection(QString connectionId, bool disable);

	signals:
		void connectionStateChanged(QString connectionId, bool state);

	private:
		::ConnectionsInfo m_buildConnections;

		std::vector<ConnectionImplPtr> m_connections;
		std::map<Hash, ConnectionImplPtr> m_connectionMap;				// ConnectionID to connection
		std::multimap<Hash, ConnectionImplPtr> m_lmToConnection;		// LM to connections
		std::map<Hash, ConnectionImplPtr> m_portToConnection;			// PortID to connection
	};

}


