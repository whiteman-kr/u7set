#include "AppDataSource.h"


// -------------------------------------------------------------------------------
//
// AppDataSource class implementation
//
// -------------------------------------------------------------------------------

AppDataSource::AppDataSource()
{
}


bool AppDataSource::getState(Network::AppDataSourceState* protoState) const
{
	if (protoState == nullptr)
	{
		assert(false);
		return false;
	}

	protoState->set_id(m_id);
	protoState->set_uptime(m_uptime);
	protoState->set_receiveddatasize(m_receivedDataSize);
	protoState->set_datareceivingrate(m_dataReceivingRate);
	protoState->set_receivedframescount(m_receivedFramesCount);
	protoState->set_processingenabled(m_dataProcessingEnabled);
	protoState->set_processedpacketcount(m_receivedPacketCount);
	protoState->set_errorprotocolversion(m_errorProtocolVersion);
	protoState->set_errorframesquantity(m_errorFramesQuantity);
	protoState->set_errorframeno(m_errorFrameNo);
	protoState->set_lostedpackets(m_lostedFramesCount);
	protoState->set_errorbadframesize(m_errorBadFrameSize);
	protoState->set_haserrors(m_hasErrors);

	return true;
}


bool AppDataSource::setState(const Network::AppDataSourceState& protoState)
{
	m_id = protoState.id();
	m_uptime = protoState.uptime();
	m_receivedDataSize = protoState.receiveddatasize();
	m_dataReceivingRate = protoState.datareceivingrate();
	m_receivedFramesCount = protoState.receivedframescount();
	m_dataProcessingEnabled = protoState.processingenabled();
	m_receivedPacketCount = protoState.processedpacketcount();
	m_errorProtocolVersion = protoState.errorprotocolversion();
	m_errorFramesQuantity = protoState.errorframesquantity();
	m_errorFrameNo = protoState.errorframeno();
	m_lostedFramesCount = protoState.lostedpackets();
	m_errorBadFrameSize = protoState.errorbadframesize();
	m_hasErrors = protoState.haserrors();

	return true;
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

void AppDataReceiver::checkDataSourcesDataReceiving()
{
	qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

	for(const AppDataSourceShared dataSource : m_appDataSourcesIP)
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



/*
 *
 *

	qDebug() << "Ideal thread count:" << QThread::idealThreadCount();


void AppDataReceiver::checkDataSourcesDataReceiving()
{
	qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

	for(const AppDataSourceShared dataSource : m_appDataSourcesIP)
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
}*/


/*
 *
 * void AppDataProcessingWorker::parseRupData()
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

	int autoArchivingGroup = sourceParseInfo->getAutoArchivingGroup(m_rupData.time.system.timeStamp);

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

		bool hasArchivingReason = signalState->setState(m_rupData.time, validity, value, autoArchivingGroup);

		if (hasArchivingReason == true)
		{
			m_signalStatesQueue.push(&signalState->stored());
		}
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
		return false;
	}

	quint16 rawValue16 = 0;
	quint32 rawValue32 = 0;

	switch(parseInfo.type)
	{
	case E::SignalType::Discrete:

		assert(parseInfo.dataSize == SIZE_1BIT);

		rawValue16 = *reinterpret_cast<quint16*>(m_rupData.data + valueOffset);

		if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
		{
			rawValue16 = reverseUint16(rawValue16);
		}

		value = static_cast<double>((rawValue16 >> bitNo) & 0x0001);

		break;

	case E::SignalType::Analog:

		assert(parseInfo.dataSize == SIZE_32BIT);
		assert(bitNo == 0);

		rawValue32 = *reinterpret_cast<quint32*>(m_rupData.data + valueOffset);

		if (parseInfo.byteOrder == E::ByteOrder::BigEndian)
		{
			rawValue32 = reverseUint32(rawValue32);
		}

		switch (parseInfo.analogSignalFormat)
		{
		case E::AnalogAppSignalFormat::Float32:
			value = static_cast<double>(*reinterpret_cast<float*>(&rawValue32));
			break;

		case E::AnalogAppSignalFormat::SignedInt32:
			value = static_cast<double>(*reinterpret_cast<qint32*>(&rawValue32));
			break;

		default:
			assert(false);
		}
		break;

	default:
		qDebug() << "Signal index (" << parseInfo.index << ") has unknown E::SignalType " << parseInfo.dataSize;
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
		validity = AppSignalState::VALID;				// no validity flags in reg buffer
		return true;
	}

	validityOffset *= 2;					// offset in Words => offset in Bytes

	if (validityOffset >= m_rupData.dataSize)
	{
		assert(false);
		validity = AppSignalState::INVALID;
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
*/
