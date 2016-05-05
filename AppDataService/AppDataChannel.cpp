#include "AppDataChannel.h"


// -------------------------------------------------------------------------------
//
// AppDataChannel class implementation
//
// -------------------------------------------------------------------------------

AppDataChannel::AppDataChannel(int channel, const HostAddressPort& dataReceivingIP) :
	DataChannel(channel, DataSource::DataType::App, dataReceivingIP)
{
}


AppDataChannel::~AppDataChannel()
{
}


void AppDataChannel::clear()
{
	m_processingThreadsPool.stopAndClearProcessingThreads();
	m_sourceParseInfoMap.clear();

	DataChannel::clear();
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


void AppDataChannel::onThreadStarted()
{
	DataChannel::onThreadStarted();

	m_processingThreadsPool.createProcessingThreads(4, m_rupDataQueue, m_sourceParseInfoMap, *m_signalStates);
	m_processingThreadsPool.startProcessingThreads();
}


void AppDataChannel::onThreadFinished()
{
	m_processingThreadsPool.stopAndClearProcessingThreads();

	DataChannel::onThreadFinished();
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
