#include "AppDataReceiver.h"
#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"


// -------------------------------------------------------------------------------
//
// AppDataChannel class implementation
//
// -------------------------------------------------------------------------------

AppDataReceiver::AppDataReceiver(const HostAddressPort& dataReceivingIP) :
	m_dataReceivingIP(dataReceivingIP),
	m_timer1s(this)
{
}


AppDataReceiver::~AppDataReceiver()
{
	clear();
}


void AppDataReceiver::prepare(AppSignals& appSignals, AppSignalStates* signalStates)
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

		SourceSignalsParseInfo* sourceParseInfo = new SourceSignalsParseInfo(m_autoArchivingGroupsCount);

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

void AppDataReceiver::addDataSource(AppDataSource* appDataSource)
{
	if (appDataSource == nullptr)
	{
		assert(false);
		return;
	}

	if (appDataSource->lmDataType() != m_dataType)
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


void AppDataReceiver::onThreadStarted()
{
	m_processingThreadsPool.createProcessingThreads(1, m_rupDataQueue, m_sourceParseInfoMap, *m_signalStates, m_signalStatesQueue);

//	m_processingThreadsPool.createProcessingThreads(4, m_rupDataQueue, m_sourceParseInfoMap, *m_signalStates);
	m_processingThreadsPool.startProcessingThreads();
	
	connect(&m_timer1s, &QTimer::timeout, this, &AppDataReceiver::onTimer1s);
	
	m_timer1s.setInterval(1000);
	m_timer1s.start();

	qDebug() << "Ideal thread count:" << QThread::idealThreadCount();
}


void AppDataReceiver::onThreadFinished()
{
	m_timer1s.stop();
	closeSocket();

	m_processingThreadsPool.stopAndClearProcessingThreads();
}


void AppDataReceiver::createAndBindSocket()
{
	if (m_socket == nullptr)
	{
		m_socket = new QUdpSocket(this);

		qDebug() << "DataChannel: listening socket created";

		connect(m_socket, &QUdpSocket::readyRead, this, &AppDataReceiver::onSocketReadyRead);
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


void AppDataReceiver::closeSocket()
{
	if (m_socket != nullptr)
	{
		m_socket->close();
		delete m_socket;
		m_socket = nullptr;
	}

	m_socketBound = false;
}


void AppDataReceiver::checkDataSourcesDataReceiving()
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


void AppDataReceiver::invalidateDataSourceSignals(quint32 dataSourceIP, qint64 currentTime)
{
	SourceSignalsParseInfo* sourceParseInfo = m_sourceParseInfoMap.value(dataSourceIP, nullptr);

	if (sourceParseInfo == nullptr)
	{
		return;
	}

	Times time;

	time.system.timeStamp = currentTime;

	for(const SignalParseInfo& parseInfo : *sourceParseInfo)
	{
		AppSignalStateEx* signalState = (*m_signalStates)[parseInfo.index];

		if (signalState == nullptr)
		{
			assert(false);
			continue;
		}

		signalState->setState(time, AppSignalState::INVALID, 0, NO_AUTOARCHIVING_GROUP);
	}

	HostAddressPort addr(dataSourceIP, 0);
	qDebug() << "Invalidate signals of source" << addr.addressStr();
}


void AppDataReceiver::clear()
{
	m_processingThreadsPool.stopAndClearProcessingThreads();
	m_sourceParseInfoMap.clear();
}


void AppDataReceiver::onTimer1s()
{
	createAndBindSocket();
	checkDataSourcesDataReceiving();

	if (m_receivedFramesCount != 0)
	{
		qDebug() << "Receive per second " << m_receivedFramesCount;

		m_receivedFramesCount = 0;
	}
}


void AppDataReceiver::onSocketReadyRead()
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

AppDataReceiverlThread::AppDataReceiverlThread(int channel,
										   const HostAddressPort& dataRecievingIP,
										   AppSignalStatesQueue& signalStatesQueue,
										   int autoArchivingGroupsCount)
{
	m_appDataChannel = new AppDataReceiver(channel, dataRecievingIP, signalStatesQueue, autoArchivingGroupsCount);
	addWorker(m_appDataChannel);
}


void AppDataReceiverlThread::prepare(AppSignals& appSignals, AppSignalStates* signalStates)
{
	if (m_appDataChannel == nullptr)
	{
		assert(false);
		return;
	}

	m_appDataChannel->prepare(appSignals, signalStates);
}



void AppDataReceiverlThread::addDataSource(AppDataSource* appDataSource)
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
