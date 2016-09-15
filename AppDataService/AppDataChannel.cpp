#include "AppDataChannel.h"
#include "../lib/AppSignalState.h"

// -------------------------------------------------------------------------------
//
// AppDataChannel class implementation
//
// -------------------------------------------------------------------------------

AppDataChannel::AppDataChannel(int channel, const HostAddressPort& dataReceivingIP) :
	m_dataType(DataSource::DataType::App),
	m_channel(channel),
	m_dataReceivingIP(dataReceivingIP),
	m_rupDataQueue(500),
	m_timer1s(this)
{
}


AppDataChannel::~AppDataChannel()
{
	clear();
}


void AppDataChannel::prepare(AppSignals& appSignals, AppSignalStates* signalStates)
{
	m_signalStates = signalStates;

	m_sourceParseInfoMap.clear();

	// scan DataSources
	//
	for(DataSource* dataSource : m_dataSources)
	{
		if (dataSource == nullptr)
		{
			assert(false);
			continue;
		}

		quint32 dataSourceIP = dataSource->lmAddress32();

		if (m_sourceParseInfoMap.contains(dataSourceIP) == true)
		{
			qDebug() <<	"Duplicate DataSource IP " << dataSource->lmAddressStr();
			assert(false);
			continue;
		}

		SourceSignalsParseInfo* sourceParseInfo = new SourceSignalsParseInfo();

		// scan signals associated with DataSource
		//
		for(const QString& assocSignalID : dataSource->associatedSignals())
		{
			if (appSignals.contains(assocSignalID) == false)
			{
				qDebug() << "Not found associated signal " << assocSignalID;
				continue;
			}

			Signal* signal = appSignals[assocSignalID];
			int index = appSignals.indexOf(assocSignalID);

			if (signal == nullptr)
			{
				assert(false);
				continue;
			}

			SignalParseInfo parceInfo;

			parceInfo.setSignalParams(index, *signal);

			sourceParseInfo->append(parceInfo);
		}

		m_sourceParseInfoMap.insert(dataSourceIP, sourceParseInfo);
	}
}


void AppDataChannel::addDataSource(DataSource* dataSource)
{
	if (dataSource == nullptr)
	{
		assert(false);
		return;
	}

	if (dataSource->lmDataType() != m_dataType ||
		dataSource->lmChannel() != m_channel)
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


void AppDataChannel::onThreadStarted()
{
	m_processingThreadsPool.createProcessingThreads(1, m_rupDataQueue, m_sourceParseInfoMap, *m_signalStates);

//	m_processingThreadsPool.createProcessingThreads(4, m_rupDataQueue, m_sourceParseInfoMap, *m_signalStates);
	m_processingThreadsPool.startProcessingThreads();
	
	connect(&m_timer1s, &QTimer::timeout, this, &AppDataChannel::onTimer1s);
	
	m_timer1s.setInterval(1000);
	m_timer1s.start();
}


void AppDataChannel::onThreadFinished()
{
	m_timer1s.stop();
	closeSocket();

	m_processingThreadsPool.stopAndClearProcessingThreads();
}


void AppDataChannel::createAndBindSocket()
{
	if (m_socket == nullptr)
	{
		m_socket = new QUdpSocket(this);

		qDebug() << "DataChannel: listening socket created";

		connect(m_socket, &QUdpSocket::readyRead, this, &AppDataChannel::onSocketReadyRead);
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


void AppDataChannel::closeSocket()
{
	if (m_socket != nullptr)
	{
		m_socket->close();
		delete m_socket;
		m_socket = nullptr;
	}

	m_socketBound = false;
}


void AppDataChannel::checkDataSourcesDataReceiving()
{
	qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

	for(DataSource* dataSource : m_dataSources)
	{
		if (dataSource == nullptr)
		{
			assert(false);
			continue;
		}

		if (dataSource->state() == DataSourceState::receiveData && (currentTime - dataSource->lastPacketTime()) > PACKET_TIMEOUT)
		{
			dataSource->setState(DataSourceState::noData);
			
			invalidateDataSourceSignals(dataSource->lmAddress32(), currentTime);
		}
	}
	
}


void AppDataChannel::invalidateDataSourceSignals(quint32 dataSourceIP, qint64 currentTime)
{
	SourceSignalsParseInfo* sourceParseInfo = m_sourceParseInfoMap.value(dataSourceIP, nullptr);

	if (sourceParseInfo == nullptr)
	{
		return;
	}

	Times time;

	time.system = currentTime;

	AppSignalStateFlags flags;

	flags.valid = INVALID_STATE;

	for(const SignalParseInfo& parseInfo : *sourceParseInfo)
	{
		AppSignalStateEx* signalState = (*m_signalStates)[parseInfo.index];

		if (signalState == nullptr)
		{
			assert(false);
			continue;
		}

		signalState->setState(time, flags, 0);
	}

	HostAddressPort addr(dataSourceIP, 0);
	qDebug() << "Invalidate signals for source" << addr.addressStr();
}


void AppDataChannel::clear()
{
	m_processingThreadsPool.stopAndClearProcessingThreads();
	m_sourceParseInfoMap.clear();
}


void AppDataChannel::onTimer1s()
{
	createAndBindSocket();
	checkDataSourcesDataReceiving();
}


void AppDataChannel::onSocketReadyRead()
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
// AppDataChannelThread class implementation
//
// -------------------------------------------------------------------------------

AppDataChannelThread::AppDataChannelThread(int channel, const HostAddressPort& dataRecievingIP)
{
	m_appDataChannel = new AppDataChannel(channel, dataRecievingIP);
	addWorker(m_appDataChannel);
}


void AppDataChannelThread::prepare(AppSignals& appSignals, AppSignalStates* signalStates)
{
	if (m_appDataChannel == nullptr)
	{
		assert(false);
		return;
	}

	m_appDataChannel->prepare(appSignals, signalStates);
}



void AppDataChannelThread::addDataSource(DataSource* dataSource)
{
	if (m_appDataChannel != nullptr)
	{
		m_appDataChannel->addDataSource(dataSource);
	}
	else
	{
		assert(false);
	}
}
