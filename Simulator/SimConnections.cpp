#include "SimConnections.h"
#include "../lib/TimeStamp.h"

namespace Sim
{
	//
	// Sim::ConnectionData
	//
	int ConnectionData::sizeBytes() const
	{
		return m_data.size();
	}

	int ConnectionData::sizeWords() const
	{
		assert(m_data.size() % 2 == 0);
		return m_data.size() / 2;
	}

	//
	// Sim::ConnectionPort
	//
	ConnectionPort::ConnectionPort(::ConnectionPortInfo portInfo) :
		m_portInfo(portInfo)
	{
	}

	const ::ConnectionPortInfo& ConnectionPort::portInfo() const
	{
		return m_portInfo;
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

			// Preallocate buffers for sending and receiving
			//
			{
				QByteArray* portReceiveBuffer = getPortReceiveBuffer(cp->portInfo().portNo);
				if (portReceiveBuffer == nullptr)
				{
					assert(portReceiveBuffer);
				}
				else
				{
					portReceiveBuffer->resize(cp->portInfo().rxDataSizeW * 2);
				}
			}

			{
				QByteArray* portSendBuffer = getPortSendBuffer(cp->portInfo().portNo);
				if (portSendBuffer == nullptr)
				{
					assert(portSendBuffer);
				}
				else
				{
					portSendBuffer->resize(cp->portInfo().txDataSizeW * 2);
				}
			}
		}

		return;
	}

	Connection::~Connection()
	{
//		qDebug() << "Connection::~Connection()";
//		qDebug() << "\tm_port1receiveBuffer " << QString::number(reinterpret_cast<quint64>(m_port1receiveBuffer.data()), 16);
//		qDebug() << "\tm_port1sendBuffer " << QString::number(reinterpret_cast<quint64>(m_port1sendBuffer.data()), 16);
//		qDebug() << "\tm_port2receiveBuffer " << QString::number(reinterpret_cast<quint64>(m_port2receiveBuffer.data()), 16);
//		qDebug() << "\tm_port2sendBuffer" << QString::number(reinterpret_cast<quint64>(m_port2sendBuffer.data()), 16);
//		qDebug() << "\tm_port1sentData" << QString::number(reinterpret_cast<quint64>(m_port1sentData.m_data.data()), 16);
//		qDebug() << "\tm_port2sentData" << QString::number(reinterpret_cast<quint64>(m_port2sentData.m_data.data()), 16);
	}

	const QString& Connection::connectionId() const
	{
		return m_buildConnection.ID;
	}

	Sim::ConnectionPortPtr Connection::portForLm(const QString& lmEquipmnetId)
	{
		for (Sim::ConnectionPortPtr p : m_ports)
		{
			if (p->portInfo().lmID == lmEquipmnetId)
			{
				return p;
			}
		}

		return {};
	}

	bool Connection::sendData(int portNo, QByteArray* data, std::chrono::microseconds currentTime)
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

				m_port1sentData.m_data.swap(*data);
				m_port1sentData.m_sentTime = currentTime;
			}
			return true;
		case 2:
			{
				QMutexLocker ml(&m_dataMutexPort2);

				m_port2sentData.m_data.swap(*data);
				m_port2sentData.m_sentTime = currentTime;
			}
			return true;
		default:
			assert(portNo == 1 || portNo == 2);
			return false;
		}
	}

	bool Connection::receiveData(int portNo,
								 QByteArray* data,
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
				}
				else
				{
					if (m_port2sentData.m_data.isEmpty() == false)
					{
						// Connection received something
						//
						data->swap(m_port2sentData.m_data);

						m_port2sentData.m_data.clear();
						m_port2sentData.m_sentTime = currentTime;		// timeout will be counted from this moment
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
				}
				else
				{
					if (m_port1sentData.m_data.isEmpty() == false)
					{
						// Connection received something
						//
						data->swap(m_port1sentData.m_data);

						m_port1sentData.m_data.clear();
						m_port1sentData.m_sentTime = currentTime;		// timeout will be counted from this moment
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
		default:
			assert(portNo == 1 || portNo == 2);
			return false;
		}
	}

	QString Connection::type() const
	{
		return m_buildConnection.type;
	}

	const ::ConnectionInfo& Connection::connectionInfo() const
	{
		return m_buildConnection;
	}

	const std::vector<Sim::ConnectionPortPtr>& Connection::ports() const
	{
		return m_ports;
	}

	bool Connection::enabled() const
	{
		return m_enable;
	}

	void Connection::setEnabled(bool value)
	{
		m_enable = value;
	}

	QByteArray* Connection::getPortReceiveBuffer(int portNo)
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

	QByteArray* Connection::getPortSendBuffer(int portNo)
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
	Connections::Connections()
	{
	}

	Connections::~Connections()
	{
		qDebug() << "Connections::~Connections()";
	}

	void Connections::clear()
	{
		m_buildConnections = {};

		m_connectionMap.clear();
		m_lmToConnection.clear();
		m_portToConnection.clear();
		m_portMap.clear();

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
			for (auto p : c->ports())
			{
				m_lmToConnection.insert({::calcHash(p->portInfo().lmID), c});
				m_portToConnection[::calcHash(p->portInfo().equipmentID)] = c;
				m_portMap[::calcHash(p->portInfo().equipmentID)] = p;
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
}
