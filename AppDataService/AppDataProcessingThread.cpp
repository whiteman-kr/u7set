#include "AppDataProcessingThread.h"


// -------------------------------------------------------------------------------
//
// AppDataProcessingWorker class implementation
//
// -------------------------------------------------------------------------------

AppDataProcessingWorker::AppDataProcessingWorker(int number, RupDataQueue& rupDataQueue, const SourceParseInfoMap& sourceParseInfoMap, AppSignalStates& signalStates) :
	m_number(number),
	m_rupDataQueue(rupDataQueue),
	m_sourceParseInfoMap(sourceParseInfoMap),
	m_signalStates(signalStates)
{
}


void AppDataProcessingWorker::onThreadStarted()
{
	connect(&m_rupDataQueue, &RupDataQueue::queueNotEmpty, this, &AppDataProcessingWorker::slot_rupDataQueueIsNotEmpty);
	qDebug() << "Processing thread started" << m_number;
}


void AppDataProcessingWorker::onThreadFinished()
{
	qDebug() << "Processing thread finished" << m_number;
}


void AppDataProcessingWorker::slot_rupDataQueueIsNotEmpty()
{
	int count = 0;

	do
	{
		bool result = m_rupDataQueue.pop(&m_rupData);

		if (result == false)
		{
			break;
		}

		parseRupData();

		count++;
	}
	while(count < 50);		//
}


void AppDataProcessingWorker::parseRupData()
{
	m_parsedRupDataCount++;

	// parse data from m_rupData
	//
	quint32 sourceIP = m_rupData.sourceIP;

	SourceSignalsParseInfo* sourceParseInfo = m_sourceParseInfoMap.value(sourceIP, nullptr);

	if (sourceParseInfo == nullptr)
	{
		m_notFoundIPCount++;
		return;
	}

	for(const SignalParseInfo& parseInfo : *sourceParseInfo)
	{
		double value = 0;

		bool result = getDoubleValue(parseInfo, value);

		if (result == false)
		{
			m_valueParsingErrorCount++;
			continue;
		}

		AppSignalStateFlags flags;

		flags.reset();

		result = getValidity(parseInfo, flags);

		if (result == false)
		{
			m_validityParsingErrorCount++;
		}

		AppSignalState* signalState = m_signalStates[parseInfo.index];

		if (signalState == nullptr)
		{
			m_badSignalStateIndexCount++;
			continue;
		}

		signalState->setState(m_rupData.time, flags, value);
	}
}


bool AppDataProcessingWorker::getDoubleValue(const SignalParseInfo& parseInfo, double& value)
{
	// get double signal value from m_rupData.data buffer using parseInfo
	//
	int valueOffset = parseInfo.valueAddr.offset() * 2;		// offset in Words => offset in Bytes

	if (valueOffset >= m_rupData.dataSize)
	{
		assert(false);
		return false;
	}

	if (parseInfo.dataSize == 1)
	{
		quint16 rawValue = *reinterpret_cast<quint16*>(m_rupData.data + valueOffset);

		if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
		{
			rawValue = qFromBigEndian<quint16>(rawValue);
		}

		value = (rawValue >> parseInfo.valueAddr.bit()) & 0x0001;
	}
	else
	{
		if (parseInfo.dataSize == 32)
		{
			assert(parseInfo.valueAddr.bit() == 0);

			quint32 rawValue = *reinterpret_cast<quint32*>(m_rupData.data + valueOffset);

			if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
			{
				rawValue = qFromBigEndian<quint32>(rawValue);
			}

			switch (parseInfo.dataFormat)
			{
			case E::DataFormat::Float:
				value = *reinterpret_cast<float*>(&rawValue);
				break;

			case E::DataFormat::SignedInt:
				value = *reinterpret_cast<qint32*>(&rawValue);
				break;

			case E::DataFormat::UnsignedInt:
				value = rawValue;
				break;
			}
		}
		else
		{
			assert(false);
			return false;
		}
	}

	return true;
}


bool AppDataProcessingWorker::getValidity(const SignalParseInfo& parseInfo, AppSignalStateFlags& flags)
{
	// get signal validity from m_rupData.data buffer using parseInfo
	//
	int validityOffset = parseInfo.validityAddr.offset();

	if (validityOffset == BAD_ADDRESS)
	{
		flags.valid = 1;				// no validity flags in reg buffer
		return true;
	}

	validityOffset *= 2;				// offset in Words => offset in Bytes

	if (validityOffset >= m_rupData.dataSize)
	{
		assert(false);
		return false;
	}

	quint16 rawValue = *reinterpret_cast<quint16*>(m_rupData.data + validityOffset);

	if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
	{
		rawValue = qFromBigEndian<quint16>(rawValue);
	}

	flags.valid = (rawValue >> parseInfo.validityAddr.bit()) & 0x0001;

	return true;
}

// -------------------------------------------------------------------------------
//
// AppDataProcessingThread class implementation
//
// -------------------------------------------------------------------------------

AppDataProcessingThread::AppDataProcessingThread(int number, RupDataQueue& rupDataQueue,
												const SourceParseInfoMap& sourceParseInfoMap,
												AppSignalStates& signalStates) :
	SimpleThread(new AppDataProcessingWorker(number, rupDataQueue, sourceParseInfoMap, signalStates))
{
}


// -------------------------------------------------------------------------------
//
// AppDataProcessingThreadsPool class implementation
//
// -------------------------------------------------------------------------------

void AppDataProcessingThreadsPool::createProcessingThreads(int poolSize, RupDataQueue& rupDataQueue,
											const SourceParseInfoMap& sourceParseInfoMap,
											AppSignalStates& signalStates)
{
	if (count() > 0)
	{
		stopAndClearProcessingThreads();
	}

	if (poolSize > 8)
	{
		poolSize = 8;
	}
	else
	{
		if (poolSize <= 0)
		{
			poolSize = 1;
		}
	}

	for(int i = 0; i < poolSize; i++)
	{
		AppDataProcessingThread* processingThread = new AppDataProcessingThread(i, rupDataQueue, sourceParseInfoMap, signalStates);

		append(processingThread);
	}
}


void AppDataProcessingThreadsPool::startProcessingThreads()
{
	for(AppDataProcessingThread* processingThread : *this)
	{
		if (processingThread == nullptr)
		{
			assert(false);
			continue;
		}

		processingThread->start();
	}
}


void AppDataProcessingThreadsPool::stopAndClearProcessingThreads()
{
	for(AppDataProcessingThread* processingThread : *this)
	{
		if (processingThread == nullptr)
		{
			assert(false);
			continue;
		}

		processingThread->quitAndWait();
		delete processingThread;
	}

	clear();
}
