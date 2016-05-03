#include "../include/DataChannel.h"


// -------------------------------------------------------------------------------
//
// DataChannel class implementation
//
// -------------------------------------------------------------------------------

DataChannel::DataChannel(int channel, DataSource::DataType dataType, const HostAddressPort& dataReceivingIP) :
	m_channel(channel),
	m_dataType(dataType),
	m_dataReceivingIP(dataReceivingIP),
	m_timer(this),
	m_rupDataQueue(500)
{
}


DataChannel::~DataChannel()
{
	clear();
}


void DataChannel::clear()
{
	for(DataSource* dataSource : m_dataSources)
	{
		delete dataSource;
	}

	m_dataSources.clear();
}


void DataChannel::addDataSource(DataSource* dataSource)
{
	if (dataSource == nullptr)
	{
		assert(false);
		return;
	}

	if (dataSource->dataType() != m_dataType ||
		dataSource->channel() != m_channel)
	{
		assert(false);
		return;
	}

	if (m_dataSources.contains(dataSource->lmAddress32()))
	{
		assert(false);
		return;
	}

	m_dataSources.insert(dataSource->lmAddress32(), dataSource);
}


void DataChannel::onThreadStarted()
{
	connect(&m_timer, &QTimer::timeout, this, &DataChannel::onTimer);

	m_timer.setInterval(1000);
	m_timer.start();

}

void DataChannel::onThreadFinished()
{
	m_timer.stop();
	closeSocket();
}


void DataChannel::closeSocket()
{
	if (m_socket != nullptr)
	{
		m_socket->close();
		delete m_socket;
		m_socket = nullptr;
	}

	m_socketBound = false;
}


void DataChannel::onTimer()
{
	createAndBindSocket();
}


void DataChannel::createAndBindSocket()
{
	if (m_socket == nullptr)
	{
		m_socket = new QUdpSocket(this);

		qDebug() << "DataChannel: listening socket created";

		connect(m_socket, &QUdpSocket::readyRead, this, &DataChannel::onSocketReadyRead);
	}

	if (m_socketBound == false)
	{
		m_socketBound = m_socket->bind(m_dataReceivingIP.address(), m_dataReceivingIP.port());

		if (m_socketBound == true)
		{
			QString str = QString("DataChannel: socket bound on %1 - OK").arg(m_dataReceivingIP.addressPortStr());
			qDebug() << str;
		}
	}
}


void DataChannel::onSocketReadyRead()
{
	if (m_socket == nullptr)
	{
		assert(false);
		return;
	}

	QHostAddress from;

	qint64 size = m_socket->pendingDatagramSize();

	if (size > sizeof(m_rupFrame))
	{
		assert(false);
		m_socket->readDatagram(reinterpret_cast<char*>(&m_rupFrame), sizeof(m_rupFrame), &from);
		qDebug() << "DataChannel: datagram too big";
		return;
	}

	qint64 result = m_socket->readDatagram(reinterpret_cast<char*>(&m_rupFrame), sizeof(m_rupFrame), &from);

	if (result == -1)
	{
		closeSocket();
		qDebug() << "DataChannel: read socket error";
	}

	assert(result == sizeof(m_rupFrame));

	quint32 ip = from.toIPv4Address();

	if (m_dataSources.contains(ip))
	{
		DataSource* dataSource = m_dataSources[ip];

		if (dataSource != nullptr)
		{
			m_rupFrame.header.toHostFormat();
			dataSource->processPacket(ip, m_rupFrame, m_rupDataQueue);
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		if (m_unknownDataSources.contains(ip) == false)
		{
			m_unknownDataSources.insert(ip, ip);
		}
	}
}


// -------------------------------------------------------------------------------
//
// DataChannelThread class implementation
//
// -------------------------------------------------------------------------------

DataChannelThread::DataChannelThread()
{
}


void DataChannelThread::addDataSource(DataSource* dataSource)
{
	if (m_dataChannel != nullptr)
	{
		m_dataChannel->addDataSource(dataSource);
	}
	else
	{
		assert(false);
	}
}


