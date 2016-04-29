#include "DataChannel.h"


// -------------------------------------------------------------------------------
//
// DataChannel class implementation
//
// -------------------------------------------------------------------------------

DataReceivingChannel::DataReceivingChannel(int channel, AppDataSource::DataType dataType, const HostAddressPort& dataReceivingIP) :
	m_channel(channel),
	m_dataType(dataType),
	m_dataReceivingIP(dataReceivingIP),
	m_timer(this)
{
}


DataReceivingChannel::~DataReceivingChannel()
{
	clear();
}


void DataReceivingChannel::clear()
{
	for(AppDataSource* dataSource : m_dataSources)
	{
		delete dataSource;
	}

	m_dataSources.clear();
}


void DataReceivingChannel::addDataSource(AppDataSource* dataSource)
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


void DataReceivingChannel::onThreadStarted()
{
	connect(&m_timer, &QTimer::timeout, this, &DataReceivingChannel::onTimer);

	m_timer.setInterval(1000);
	m_timer.start();

}

void DataReceivingChannel::onThreadFinished()
{
	m_timer.stop();
	closeSocket();
}


void DataReceivingChannel::closeSocket()
{
	if (m_socket != nullptr)
	{
		m_socket->close();
		delete m_socket;
		m_socket = nullptr;
	}

	m_socketBound = false;
}


void DataReceivingChannel::onTimer()
{
	createAndBindSocket();
}


void DataReceivingChannel::createAndBindSocket()
{
	if (m_socket == nullptr)
	{
		m_socket = new QUdpSocket(this);

		qDebug() << "DataChannel: listening socket created";

		connect(m_socket, &QUdpSocket::readyRead, this, &DataReceivingChannel::onSocketReadyRead);
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


void DataReceivingChannel::onSocketReadyRead()
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
		AppDataSource* dataSource = m_dataSources[ip];

		if (dataSource != nullptr)
		{
			dataSource->processPacket(ip, m_rupFrame);
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		m_unknownDataSources.insert(ip, ip);
	}

}



// -------------------------------------------------------------------------------
//
// AppDataChannel class implementation
//
// -------------------------------------------------------------------------------

AppDataReceivingChannel::AppDataReceivingChannel(int channel, const HostAddressPort& dataReceivingIP) :
	DataReceivingChannel(channel, AppDataSource::DataType::App, dataReceivingIP)
{
}



// -------------------------------------------------------------------------------
//
// DataChannelThread class implementation
//
// -------------------------------------------------------------------------------

DataChannelThread::DataChannelThread(int channel, AppDataSource::DataType dataType, const HostAddressPort& dataRecievingIP)
{
	switch (dataType)
	{
	case AppDataSource::DataType::App:
		m_dataChannel = new AppDataReceivingChannel(channel, dataRecievingIP);
		break;

	case AppDataSource::DataType::Diag:
	//	m_dataChannel = new DiagDataChannel(channel, dataRecievingIP);
		break;

	default:
		assert(false);
		break;
	}

	if (m_dataChannel != nullptr)
	{
		addWorker(m_dataChannel);
	}
}


void DataChannelThread::addDataSource(AppDataSource* dataSource)
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
