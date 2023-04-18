#include "SimConnections.h"


namespace Sim
{
	//
	// Sim::ConnectionData
	//
	int ConnectionData::sizeBytes() const
	{
		return static_cast<int>(m_data.size());
	}

	int ConnectionData::sizeWords() const
	{
		assert(m_data.size() % 2 == 0);
		return static_cast<int>(m_data.size() / 2);
	}


	//
	// Sim::Connection
	//
	Connection::Connection(const ConnectionInfo& buildConnection) :
		m_buildConnection(buildConnection)
	{
		auto portPrepare = [this](ConnectionPort& cp)
		{
			// Preallocate buffers for sending and receiving
			//
			{
				std::vector<char>* portReceiveBuffer = getPortReceiveBuffer(cp.portInfo().portNo);
				if (portReceiveBuffer == nullptr)
				{
					assert(portReceiveBuffer);
				}
				else
				{
					portReceiveBuffer->resize(cp.portInfo().rxDataSizeW * 2);
				}
			}

			{
				std::vector<char>* portSendBuffer = getPortSendBuffer(cp.portInfo().portNo);
				if (portSendBuffer == nullptr)
				{
					assert(portSendBuffer);
				}
				else
				{
					portSendBuffer->resize(cp.portInfo().txDataSizeW * 2);
				}
			}
		};

		Q_ASSERT(m_buildConnection.ports.size() <= 2);

		if (m_buildConnection.ports.size() > 0)
		{
			m_port1 = ConnectionPort{m_buildConnection.ports[0]};
			portPrepare(m_port1.value());
		}

		if (m_buildConnection.ports.size() > 1)
		{
			m_port2 = ConnectionPort{m_buildConnection.ports[1]};
			portPrepare(m_port2.value());
		}

		return;
	}

	const QString& Connection::connectionId() const
	{
		return m_buildConnection.ID;
	}

	const Sim::ConnectionPort* Connection::portForLmRawPtr(const QString& lmEquipmnetId) const
	{
		return portForLmRawPtr(calcHash(lmEquipmnetId));
	}

	const Sim::ConnectionPort* Connection::portForLmRawPtr(Hash logicModuleIdHash) const
	{
		const Sim::ConnectionPort* result = nullptr;

		if (m_port1.has_value() == true && m_port1->lmIdHash() == logicModuleIdHash)
		{
			result = &m_port1.value();
		}

		if (m_port2.has_value() == true && m_port2->lmIdHash() == logicModuleIdHash)
		{
			result = &m_port2.value();
		}

		return result;
	}

	Sim::ConnectionPort* Connection::portForLmRawPtr(Hash logicModuleIdHash)
	{
		Sim::ConnectionPort* result = nullptr;

		if (m_port1.has_value() == true && m_port1->lmIdHash() == logicModuleIdHash)
		{
			result = &m_port1.value();
		}

		if (m_port2.has_value() == true && m_port2->lmIdHash() == logicModuleIdHash)
		{
			result = &m_port2.value();
		}

		return result;
	}

	bool Connection::sendData(int portNo,
							  std::vector<char>* data,
							  std::chrono::microseconds currentTime)
	{
		if (data == nullptr)
		{
			assert(data);
			return false;
		}

		switch (portNo)
		{
		case 1:
			{
				QMutexLocker ml(&m_dataMutexPort1);

				std::swap(m_port1sentData.m_data, *data);
				m_port1sentData.m_sentTime = currentTime;
			}
			return true;
		case 2:
			{
				QMutexLocker ml(&m_dataMutexPort2);

				std::swap(m_port2sentData.m_data, *data);
				m_port2sentData.m_sentTime = currentTime;
			}
			return true;
		default:
			assert(portNo == 1 || portNo == 2);
			return false;
		}
	}

	bool Connection::receiveData(int portNo,
								 std::vector<char>* data,
								 std::chrono::microseconds currentTime,
								 std::chrono::microseconds timeout,
								 bool* timeoutHappend)
	{
		if (data == nullptr || timeoutHappend == nullptr)
		{
			assert(data);
			assert(timeoutHappend);
			return false;
		}

		data->clear();
		*timeoutHappend = false;

		// There is no simulation for single port connection yet
		// may be later will be added some input file of something else
		// Just return time out
		//
		if (m_buildConnection.type == Hardware::Connection::Type::SinglePort)
		{
			*timeoutHappend = true;
			m_timeout.store(true, std::memory_order::relaxed);
			return true;
		}

		Q_ASSERT(m_buildConnection.type == Hardware::Connection::Type::PortToPort);

		switch (portNo)
		{
		case 1:
			{
				// For port 1 get data from port 2
				//
				QMutexLocker ml(&m_dataMutexPort2);

				if (currentTime - m_port2sentData.m_sentTime > timeout)
				{
					//qDebug() << "Connection::receiveData: port2 from timeout " << (currentTime - m_port2sentData.m_sentTime).count() / 1000;

					data->clear();
					*timeoutHappend = true;

					m_timeout.store(true, std::memory_order::relaxed);
				}
				else
				{
					if (m_port2sentData.m_data.empty() == false)
					{
						// Connection received something
						//
						data->swap(m_port2sentData.m_data);

						m_port2sentData.m_data.clear();
						m_port2sentData.m_sentTime = currentTime;		// timeout will be counted from this moment

						m_timeout.store(false, std::memory_order::relaxed);
					}
					else
					{
						// No new data since last call
						// just wait for timeout
						//
					}
				}
			}
			return true;
		case 2:
			{
				// For port 2 get data from port 1
				//
				QMutexLocker ml(&m_dataMutexPort1);

				if (currentTime - m_port1sentData.m_sentTime > timeout)
				{
					//qDebug() << "Connection::receiveData: port1 from timeout " << (currentTime - m_port1sentData.m_sentTime).count() / 1000;

					data->clear();
					*timeoutHappend = true;

					m_timeout.store(true, std::memory_order::relaxed);
				}
				else
				{
					if (m_port1sentData.m_data.empty() == false)
					{
						// Connection received something
						//
						data->swap(m_port1sentData.m_data);

						m_port1sentData.m_data.clear();
						m_port1sentData.m_sentTime = currentTime;		// timeout will be counted from this moment

						m_timeout.store(false, std::memory_order::relaxed);
					}
					else
					{
						// No new data since last call
						// just wait for timeout
						//
					}
				}
			}
			return true;
		}

		assert(portNo == 1 || portNo == 2);
		return false;
	}

	QString Connection::typeStr() const
	{
		return m_buildConnection.typeStr;
	}

	Hardware::Connection::Type Connection::type() const
	{
		return m_buildConnection.type;
	}

	const ::ConnectionInfo& Connection::connectionInfo() const
	{
		return m_buildConnection;
	}

	std::vector<Sim::ConnectionPort> Connection::ports() const
	{
		std::vector<Sim::ConnectionPort> result;
		result.reserve(2);

		if (m_port1.has_value() == true)
		{
			result.push_back(m_port1.value());
		}

		if (m_port2.has_value() == true)
		{
			result.push_back(m_port2.value());
		}

		return result;
	}

	bool Connection::enabled() const
	{
		return m_enable.load(std::memory_order::relaxed);
	}

	void Connection::setEnabled(bool value)
	{
		m_enable.store(value, std::memory_order::relaxed);
	}

	bool Connection::timeout() const
	{
		return m_timeout.load(std::memory_order::relaxed);
	}

	std::vector<char>* Connection::getPortReceiveBuffer(int portNo)
	{
		switch (portNo)
		{
		case 1:
			return &m_port1receiveBuffer;
		case 2:
			return &m_port2receiveBuffer;
		default:
			assert(portNo == 1 || portNo == 2);
			return nullptr;
		}
	}

	std::vector<char>* Connection::getPortSendBuffer(int portNo)
	{
		switch (portNo)
		{
		case 1:
			return &m_port1sendBuffer;
		case 2:
			return &m_port2sendBuffer;
		default:
			assert(portNo == 1 || portNo == 2);
			return nullptr;
		}
	}

	//
	// Sim::Connections
	//
	Connections::Connections(QObject* parent) :
		QObject(parent)
	{
	}

	void Connections::clear()
	{
		m_buildConnections = {};

		m_connectionMap.clear();
		m_lmToConnection.clear();
		m_portToConnection.clear();

		m_connections.clear();

		return;
	}

	bool Connections::load(QString fileName, QString* errorMessage)
	{
		assert(errorMessage);
		clear();

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

			m_connectionMap[::calcHash(c->connectionId())] = c;

			// m_lmToConnection
			//
			for (const auto& p : c->ports())
			{
				m_lmToConnection.insert({::calcHash(p.portInfo().lmID), c});
				m_portToConnection[::calcHash(p.portInfo().equipmentID)] = c;
			}
		}

		return ok;
	}

	ConnectionPtr Connections::connection(QString connectionId) const
	{
		ConnectionPtr result;

		auto it = m_connectionMap.find(::calcHash(connectionId));
		if (it != m_connectionMap.end())
		{
			result = it->second;
		}

		return result;
	}

	std::vector<ConnectionPtr> Connections::connections() const
	{
		return m_connections;
	}

	std::vector<ConnectionPtr> Connections::lmConnections(const QString& lmEquipmentId) const
	{
		Hash h = ::calcHash(lmEquipmentId);

		auto range = m_lmToConnection.equal_range(h);

		std::vector<ConnectionPtr> result;
		result.reserve(std::distance(range.first, range.second));

		for (auto i = range.first; i != range.second; ++i)
		{
			result.push_back(i->second);
		}

		return result;
	}

	void Connections::enableConnection(QString connectionId, bool enable)
	{
		auto c = connection(connectionId);
		if (c != nullptr && c->enabled() != enable)
		{
			c->setEnabled(enable);
			emit connectionStateChanged(connectionId, enable);
		}

		return;
	}

	void Connections::disableConnection(QString connectionId, bool disable)
	{
		return enableConnection(connectionId, !disable);
	}
}
