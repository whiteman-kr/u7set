#include "AppDataChannel.h"
#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"


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
	for(DataSource* dataSource : m_appDataSources)
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


void AppDataChannel::addDataSource(AppDataSource* appDataSource)
{
	if (appDataSource == nullptr)
	{
		assert(false);
		return;
	}

	if (appDataSource->lmDataType() != m_dataType ||
		appDataSource->lmChannel() != m_channel)
	{
		assert(false);
		return;
	}

	if (m_appDataSources.contains(appDataSource->lmAddress32()))
	{
		assert(false);
		return;
	}

	m_appDataSources.insert(appDataSource->lmAddress32(), appDataSource);
}


void AppDataChannel::onThreadStarted()
{
	m_processingThreadsPool.createProcessingThreads(1, m_rupDataQueue, m_sourceParseInfoMap, *m_signalStates);

//	m_processingThreadsPool.createProcessingThreads(4, m_rupDataQueue, m_sourceParseInfoMap, *m_signalStates);
	m_processingThreadsPool.startProcessingThreads();
	
	connect(&m_timer1s, &QTimer::timeout, this, &AppDataChannel::onTimer1s);
	
	m_timer1s.setInterval(1000);
	m_timer1s.start();

	qDebug() << "Ideal thread count:" << QThread::idealThreadCount();
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
			qDebug() << C_STR(str);
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

	for(DataSource* dataSource : m_appDataSources)
	{
		if (dataSource == nullptr)
		{
			assert(false);
			continue;
		}

		if (dataSource->state() == E::DataSourceState::ReceiveData && (currentTime - dataSource->lastPacketTime()) > PACKET_TIMEOUT)
		{
			dataSource->setState(E::DataSourceState::NoData);
			
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

	for(const SignalParseInfo& parseInfo : *sourceParseInfo)
	{
		AppSignalStateEx* signalState = (*m_signalStates)[parseInfo.index];

		if (signalState == nullptr)
		{
			assert(false);
			continue;
		}

		signalState->setState(time, INVALID_STATE, 0);
	}

	HostAddressPort addr(dataSourceIP, 0);
	qDebug() << "Invalidate signals of source" << addr.addressStr();
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

	if (m_receivedFramesCount != 0)
	{
		qDebug() << "Receive per second " << m_receivedFramesCount;

		m_receivedFramesCount = 0;
	}
}


void AppDataChannel::onSocketReadyRead()
{
	if (m_socket == nullptr)
	{
		assert(false);
		return;
	}

	QHostAddress from;

	for(int i = 0; i < 1000; i++)
	{
		qint64 size = m_socket->pendingDatagramSize();

		if (size == -1)
		{
			break;
		}

		qint64 result = m_socket->readDatagram(reinterpret_cast<char*>(&m_rupFrame), sizeof(m_rupFrame), &from);

		if (result == -1)
		{
			closeSocket();
			qDebug() << "DataChannel" << C_STR(m_dataReceivingIP.addressPortStr()) << " read socket error";
			return;
		}

		m_receivedFramesCount++;

		quint32 ip = from.toIPv4Address();

		AppDataSource* dataSource = m_appDataSources.value(ip, nullptr);

		if (dataSource != nullptr)
		{
			if (size != sizeof(m_rupFrame))
			{
				dataSource->incBadFrameSizeError();
			}
			else
			{
				//dataSource->pushRupFrame(m_rupFrame);
				dataSource->processPacket(ip, m_rupFrame, m_rupDataQueue);
			}
		}
		else
		{
			if (m_unknownDataSources.contains(ip) == false && m_unknownDataSources.count() < 500)
			{
				m_unknownDataSources.insert(ip, ip);
			}
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



void AppDataChannelThread::addDataSource(AppDataSource* appDataSource)
{
	if (m_appDataChannel != nullptr)
	{
		m_appDataChannel->addDataSource(appDataSource);
	}
	else
	{
		assert(false);
	}
}
