#include "AppDataProcessingThread.h"
#include "../lib/WUtils.h"


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
	while(count < 500);
}


void AppDataProcessingWorker::parseRupData()
{
	m_parsedRupDataCount++;

	if ((m_parsedRupDataCount % 500) == 0)
	{
		qDebug() << "Parced" << m_parsedRupDataCount;
	}

	// parse data from m_rupData
	//
	quint32 sourceIP = m_rupData.sourceIP;

	SourceSignalsParseInfo* sourceParseInfo = m_sourceParseInfoMap.value(sourceIP, nullptr);

	if (sourceParseInfo == nullptr)
	{
		m_notFoundIPCount++;
		return;
	}

	quint32 validity = 0;
	double value = 0;
	bool result = true;

	for(const SignalParseInfo& parseInfo : *sourceParseInfo)
	{
		if (parseInfo.valueAddr.offset() == -1)
		{
			continue;
		}

		result = getDoubleValue(parseInfo, value);

		if (result == false)
		{
			m_valueParsingErrorCount++;
			continue;
		}

		result = getValidity(parseInfo, validity);

		if (result == false)
		{
			m_validityParsingErrorCount++;
			continue;
		}

		AppSignalStateEx* signalState = m_signalStates[parseInfo.index];

		if (signalState == nullptr)
		{
			m_badSignalStateIndexCount++;
			continue;
		}

		signalState->setState(m_rupData.time, validity, value);
	}
}


bool AppDataProcessingWorker::getDoubleValue(const SignalParseInfo& parseInfo, double& value)
{
	// get double signal value from m_rupData.data buffer using parseInfo
	//
	int valueOffset = parseInfo.valueAddr.offset() * 2;		// offset in Words => offset in Bytes
	int bitNo = parseInfo.valueAddr.bit();

	if (m_rupData.dataSize > (Rup::FRAME_DATA_SIZE * Rup::MAX_FRAME_COUNT) ||
		valueOffset < 0 ||
		valueOffset >= m_rupData.dataSize ||
		bitNo <0 ||
		bitNo >= SIZE_16BIT)
	{
//		assert(false);
		return false;
	}

	quint16 rawValue16 = 0;
	quint32 rawValue32 = 0;

	switch(parseInfo.dataSize)
	{
	case SIZE_1BIT:
		rawValue16 = *reinterpret_cast<quint16*>(m_rupData.data + valueOffset);

		if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
		{
			rawValue16 = reverseUint16(rawValue16);
		}


		value = static_cast<double>((rawValue16 >> bitNo) & 0x0001);

		break;

	case SIZE_32BIT:
		assert(bitNo == 0);

		rawValue32 = *reinterpret_cast<quint32*>(m_rupData.data + valueOffset);

		if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
		{
			rawValue32 = reverseUint32(rawValue32);
		}

		switch (static_cast<E::DataFormat>(parseInfo.analogSignalFormat))
		{
		case E::DataFormat::Float:
			value = static_cast<double>(*reinterpret_cast<float*>(&rawValue32));
			break;

		case E::DataFormat::SignedInt:
			value = static_cast<double>(*reinterpret_cast<qint32*>(&rawValue32));
			break;

		case E::DataFormat::UnsignedInt:
			value = static_cast<double>(rawValue32);
			break;

		default:
			assert(false);
		}
		break;

	default:
		qDebug() << "Signal index (" << parseInfo.index << ") has dataSize = " << parseInfo.dataSize;
//		assert(false);
		return false;
	}

	return true;
}


bool AppDataProcessingWorker::getValidity(const SignalParseInfo& parseInfo, quint32& validity)
{
	// get signal validity from m_rupData.data buffer using parseInfo
	//
	int validityOffset = parseInfo.validityAddr.offset();

	if (validityOffset == BAD_ADDRESS)
	{
		validity = VALID_STATE;				// no validity flags in reg buffer
		return true;
	}

	validityOffset *= 2;					// offset in Words => offset in Bytes

	if (validityOffset >= m_rupData.dataSize)
	{
		assert(false);
		validity = INVALID_STATE;
		return false;
	}

	quint16 rawValue = *reinterpret_cast<quint16*>(m_rupData.data + validityOffset);

	if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
	{
		rawValue = qFromBigEndian<quint16>(rawValue);
	}

	validity = static_cast<quint32>((rawValue >> parseInfo.validityAddr.bit()) & 0x0001);

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
