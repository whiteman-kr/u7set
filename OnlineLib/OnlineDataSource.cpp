#include "OnlineDataSource.h"

// ---------------------------------------------------------------------------------------------
//
// OnlineDataSource class implementation
//
// ---------------------------------------------------------------------------------------------

OnlineDataSource::OnlineDataSource(const DataSource& dataSource, E::LanControllerType srcType) :
	DataSource(dataSource),
	m_sourceType(srcType),
	m_writeBufferIndex(PARSING_BUFFERS_COUNT),
	m_readBufferIndex(PARSING_BUFFERS_COUNT)
{
	Q_ASSERT(m_sourceType == E::LanControllerType::AppData ||
			 m_sourceType == E::LanControllerType::DiagData);

	switch(m_sourceType)
	{
	case E::LanControllerType::AppData:

		m_expectedDataUID = rupAppDataUID();
		initParsingBuffers(appDataFramesQuantity());
		m_acquiredSignalsCount = dataSource.appSignalsCount();

		break;

	case E::LanControllerType::DiagData:

		m_expectedDataUID = rupDiagDataUID();
		initParsingBuffers(diagDataFramesQuantity());
		m_acquiredSignalsCount = dataSource.diagSignalsCount();

		break;

	default:
		Q_ASSERT(false);		// mixed LanControllerType is not allowed here!
	}

	m_workcycle_ms = moduleWorkcycle_ms();
}

OnlineDataSource::~OnlineDataSource()
{
	clearParsingBuffers();
}

bool OnlineDataSource::initParsingBuffers(int framesQuantity)
{
	clearParsingBuffers();

	m_parsingBuffers.reserve(PARSING_BUFFERS_COUNT);

	for(int i = 0; i < PARSING_BUFFERS_COUNT; i++)
	{
		ParsingBuffer* pb = new ParsingBuffer;
		pb->allocate(framesQuantity);

		m_parsingBuffers.push_back(pb);
	}

	return true;
}

void OnlineDataSource::clearParsingBuffers()
{
	for(ParsingBuffer* pb : m_parsingBuffers)
	{
		DELETE_IF_NOT_NULL(pb);
	}

	m_parsingBuffers.clear();
}

bool OnlineDataSource::pushRupFrame(quint32 sourceIP,
									qint64 serverTime,
									bool isSimFrame,
									Rup::Frame& rupFrame,
									const QThread* thread)
{
	Q_UNUSED(sourceIP);

	if (m_dataProcessingEnabled == false)
	{
		false;
	}

	m_receivedFramesCount++;
	m_receivedDataSize += (isSimFrame == true ? sizeof(Rup::SimFrame) : sizeof(Rup::Frame));

	//

	ParsingBuffer& writeBuffer = *m_parsingBuffers[m_writeBufferIndex];

	if (writeBuffer.readyToParsing == true)
	{
		if (moveToNextWriteBuffer(thread) == false)
		{
			m_lostPacketCount++;
			return true;
		}
	}

	//

	rupFrame.header.reverseBytes();

	if (rupFrame.header.protocolVersion != rupVersion())
	{
		m_errorProtocolVersion++;
		return false;
	}

	//


	if (rupFrame.header.framesQuantity != writeBuffer.framesQuantity)
	{
		Q_ASSERT(false);
		m_errorFramesQuantity++;
		return false;
	}

	//

	int frameNo = rupFrame.header.frameNumber;

	if (frameNo >= writeBuffer.framesQuantity)
	{
		Q_ASSERT(false);
		m_errorFrameNo++;
		return false;
	}

	//

	m_receivedDataUID = rupFrame.header.dataId;

	if (m_receivedDataUID != m_expectedDataUID)
	{
		m_errorDataID++;

		if ((m_errorDataID % 600) == 0)
		{
			qDebug() << C_STR(QString("%1 wrong data UID, expected 0x%2 received 0x%3").
								arg(moduleEquipmentID()).
								arg(m_expectedDataUID, 8, 16).
								arg(m_receivedDataUID, 8, 16));
		}
		return false;
	}

	//

	bool readyToParsing = writeBuffer.copyRupFrame(frameNo, serverTime, isSimFrame, rupFrame);

	if (readyToParsing == true)
	{
		moveToNextWriteBuffer(thread);
	}

	return readyToParsing;
}

bool OnlineDataSource::updateStatistics_500ms(bool oneSecond)
{
	bool invalidateSignals = false;

	qint64 now = QDateTime::currentMSecsSinceEpoch();

	if (m_lastPacketServerTime != 0 &&
		(now - m_lastPacketServerTime > APP_DATA_SOURCE_TIMEOUT))
	{
		m_rupTimes = m_lastRupTimes;
		m_rupTimes += moduleWorkcycle_ms();

		if (m_receivesData == true)
		{
			invalidateSignals = true;
		}

		m_receivesData = false;

		clearStatistics();
	}
	else
	{
		if (oneSecond == true)
		{
			// Bytes per second
			//
			m_dataReceivingSpeed = static_cast<double>(m_receivedDataSize - m_prevReceivedDataSize);
			m_prevReceivedDataSize.store(m_receivedDataSize);
		}
	}

	if (m_firstPacketServerTime == 0 || m_lastPacketServerTime == 0)
	{
		m_uptime = 0;
	}
	else
	{
		m_uptime = (m_lastPacketServerTime - m_firstPacketServerTime) / 1000;
	}

	return invalidateSignals;
}

bool OnlineDataSource::parseNextBuffer(const QThread* thread)
{


//	CHECK RUP FRAMES CRC!!!!

//	crcOk = simFrame.rupFrame.checkCRC64();

//	if (crcOk == false)
//	{
//		m_errRupFrameCRC++;
//	}


	int ctr = 0;

	do
	{
		if (moveToNextReadBuffer(thread) == false)
		{
			break;
		}

		//

		ParsingBuffer& readBuffer = *m_parsingBuffers[m_readBufferIndex];

		if (readBuffer.readyToParsing == false)
		{
			Q_ASSERT(false);
			return false;
		}

		m_receivesData = true;

		m_receivedPacketCount++;

		const Rup::Header& header = readBuffer.frame0Header();

		bool disableTimeCorrection = false;

		if (m_rupFrameNumerator != -1 &&
			((m_rupFrameNumerator + 1) & 0xFFFF) != header.numerator)
		{
			if (header.numerator > m_rupFrameNumerator)
			{
				m_lostPacketCount += header.numerator - m_rupFrameNumerator - 1;
			}
			else
			{
				m_lostPacketCount += 0xFFFF - m_rupFrameNumerator + header.numerator - 1;
			}

			disableTimeCorrection = true;		// no sequential packets, disable time correction
		}

		m_rupFrameNumerator = header.numerator;

		qint64 timeWithoutCorrection = readBuffer.frame0ServerTime;
		qint64 dt = timeWithoutCorrection - m_lastPacketServerTime;

		if (dt == 0)
		{
			// always do correction
			//
			m_lastPacketServerTime += 1;
		}
		else
		{
			if (disableTimeCorrection == true ||
				dt > 50 ||
				dt <= (moduleWorkcycle_ms() + 1))
			{
				// NO time correction
				//
				m_lastPacketServerTime = timeWithoutCorrection;
			}
			else
			{
				// time correction
				//
				m_lastPacketServerTime += moduleWorkcycle_ms();
			}
		}

		if (m_firstPacketServerTime == 0)
		{
			m_firstPacketServerTime = m_lastPacketServerTime;
		}

		//

		QDateTime plantTime;

		const Rup::TimeStamp& timeStamp = header.timeStamp;

		// don't delete this to prevent plantTime conversion from Local to UTC time during call plantTime.toMSecsSinceEpoch()!!!
		//
		plantTime.setTimeSpec(Qt::UTC);

		plantTime.setDate(QDate(timeStamp.year, timeStamp.month, timeStamp.day));
		plantTime.setTime(QTime(timeStamp.hour, timeStamp.minute, timeStamp.second, timeStamp.millisecond));

		QDateTime localTime = QDateTime::fromMSecsSinceEpoch(m_lastPacketServerTime);

		// don't delete this to prevent localTime conversion from Local to UTC time during call localTime.toMSecsSinceEpoch()!!!
		//
		localTime.setTimeSpec(Qt::UTC);

		//

		m_rupTimes.plant.timeStamp = plantTime.toMSecsSinceEpoch();
		m_rupTimes.system.timeStamp = m_lastPacketServerTime;
		m_rupTimes.local.timeStamp = localTime.toMSecsSinceEpoch();

		m_lmTime = m_rupTimes.plant.timeStamp;

		checkPlantTime(header.timeStamp);

		m_lastRupTimes = m_rupTimes;

		//

		quint16 packetNo = header.numerator;
		bool isSimPacket = readBuffer.isSimPacket;
		const char* rupData = readBuffer.rupData();
		int rupDataSize = readBuffer.rupDataSize();

		parseRupData(m_rupTimes, isSimPacket, packetNo, rupData, rupDataSize, thread);

		ctr++;
	}
	while(ctr < 50);

	return true;
}

bool OnlineDataSource::parseRupData(const Times& time,
									bool isSimPacket,
									quint16 packetNo,
									const char* rupData,
									int rupDataSize,
									const QThread* thread)
{
	Q_UNUSED(time);
	Q_UNUSED(isSimPacket);
	Q_UNUSED(packetNo);
	Q_UNUSED(rupData);
	Q_UNUSED(rupDataSize);
	Q_UNUSED(thread);
	return true;
}

bool OnlineDataSource::invalidateAllSignals(const QThread* thread)
{
	Q_UNUSED(thread);
	return true;
}

void OnlineDataSource::checkPlantTime(const Rup::TimeStamp& plantTimeStamp)
{
	if (plantTimeStamp.year < 2000 || plantTimeStamp.year > 2500 ||
		plantTimeStamp.month < 1 || plantTimeStamp.month > 12 ||
		plantTimeStamp.day < 1 || plantTimeStamp.day > 31 ||
		plantTimeStamp.hour > 23 || plantTimeStamp.minute > 59 ||
		plantTimeStamp.second > 59 || plantTimeStamp.millisecond > 999)
	{
		m_errorPlantTimeFormat++;

		if (m_timeErrLog != nullptr)
		{
			DEBUG_LOG_ERR(m_timeErrLog, QString("Source %1 time format error %2").
											arg(moduleEquipmentID()).
											arg(getTimeStr(plantTimeStamp)));
		}
	}

	if (m_lastRupTimes.plant.timeStamp == m_rupTimes.plant.timeStamp)
	{
		m_errorDuplicatePlantTime++;

		if (m_timeErrLog != nullptr)
		{
			DEBUG_LOG_ERR(m_timeErrLog, QString("Source %1 duplicate time %2").
											arg(moduleEquipmentID()).
											arg(getTimeStr(plantTimeStamp)));
		}
	}

	if (m_lastRupTimes.plant.timeStamp > m_rupTimes.plant.timeStamp)
	{
		m_errorNonmonotonicPlantTime++;

		if (m_timeErrLog != nullptr)
		{
			DEBUG_LOG_ERR(m_timeErrLog, QString("Source %1 non monotonic time %2 (prev time %3)").
											arg(moduleEquipmentID()).
											arg(getTimeStr(plantTimeStamp)).
											arg(getTimeStr(m_lastRupTimes.plant.timeStamp)));
		}
	}

	m_lastRupTimes = m_rupTimes;
}

bool OnlineDataSource::takeProcessingOwnership(const QThread* processingThread)
{
	const QThread* expected = nullptr;

	bool result = m_processingOwner.compare_exchange_strong(expected,  processingThread);

	// if ownership has been taken by processingWorker - function returns TRUE
	//
	// result == FALSE is Ok, means that another thread is already take ownership

	return result;
}

bool OnlineDataSource::releaseProcessingOwnership(const QThread* processingThread)
{
	bool result = m_processingOwner.compare_exchange_strong(processingThread,  nullptr);

	assert(result == true);	// releaseProcessingOwnership must be called by processingWorker == m_processingOwner only !!!

	return result;
}

QString OnlineDataSource::stateStr() const
{
	return (m_receivesData ? "Receive data" : "No data");
}

QString OnlineDataSource::rupFramePlantTimeStr() const
{
	return getTimeStr(m_lmTime);
}

QString OnlineDataSource::getTimeStr(qint64 timeMs)
{
	QDateTime dt = QDateTime::fromMSecsSinceEpoch(timeMs, Qt::UTC, 0);

	QDate date = dt.date();
	QTime time = dt.time();

	return 	QString(FormatStr::DATE_TIME_FORMAT_STR).
				arg(time.hour(), 2, 10, QLatin1Char('0')).
				arg(time.minute(), 2, 10, QLatin1Char('0')).
				arg(time.second(), 2, 10, QLatin1Char('0')).
				arg(time.msec(), 3, 10, QLatin1Char('0')).
				arg(date.day(), 2, 10, QLatin1Char('0')).
				arg(date.month(), 2, 10, QLatin1Char('0')).
				arg(date.year(), 4, 10, QLatin1Char('0'));
}

QString OnlineDataSource::getTimeStr(const Rup::TimeStamp& ts)
{
	return 	QString(FormatStr::DATE_TIME_FORMAT_STR).
				arg(ts.hour, 2, 10, QLatin1Char('0')).
				arg(ts.minute, 2, 10, QLatin1Char('0')).
				arg(ts.second, 2, 10, QLatin1Char('0')).
				arg(ts.millisecond, 3, 10, QLatin1Char('0')).
				arg(ts.day, 2, 10, QLatin1Char('0')).
				arg(ts.month, 2, 10, QLatin1Char('0')).
				arg(ts.year, 4, 10, QLatin1Char('0'));
}

void OnlineDataSource::clearStatistics()
{
	m_dataReceivingSpeed = 0;
	m_receivedDataSize = 0;
	m_prevReceivedDataSize = 0;

	m_uptime = 0;										// in seconds!
	m_receivedDataUID = 0;

	m_lmTime = 0;
	m_rupFrameNumerator = -1;			// qint64 is Ok!

	m_dataReceivingSpeed = 0;
	m_receivedDataSize = 0;
	m_prevReceivedDataSize = 0;
	m_receivedFramesCount = 0;
	m_receivedPacketCount = 0;
	m_lostPacketCount = 0;

	m_errorProtocolVersion = 0;
	m_errorFramesQuantity = 0;
	m_errorFrameNo = 0;
	m_errorFrameCRC = 0;
	m_errorDataID = 0;

	m_errorDuplicatePlantTime = 0;
	m_errorNonmonotonicPlantTime = 0;
	m_errorPlantTimeFormat = 0;

	m_firstPacketServerTime = 0;
	m_lastPacketServerTime = 0;
}

bool OnlineDataSource::moveToNextWriteBuffer(const QThread* thread)
{
	SimpleMutexLocker locker(&m_parsingBuffersMutex, thread);

	m_parsingBuffers[m_writeBufferIndex]->readyToParsing = true;

	m_writeBufferIndex++;

	if (m_writeBufferIndex == m_readBufferIndex ||
		m_parsingBuffers[m_writeBufferIndex]->readyToParsing == true)		// buffer is not parsed yet
	{
		m_writeBufferIndex--;				// return to prev writeIndexValue
		return false;
	}

	return true;
}

bool OnlineDataSource::moveToNextReadBuffer(const QThread* thread)
{
	SimpleMutexLocker locker(&m_parsingBuffersMutex, thread);

	if (m_parsingBuffers[m_readBufferIndex]->readyToParsing == true)
	{
		return true;
	}

	m_readBufferIndex++;

	if (m_readBufferIndex == m_writeBufferIndex ||
		m_parsingBuffers[m_readBufferIndex]->readyToParsing == false)	// buffer is not ready to parsing yet
	{
		m_readBufferIndex--;				// return to prev readIndexValue
		return false;
	}

	return true;							// buffer ready to parsing
}


OnlineDataSource::ParsingBuffer::ParsingBuffer()
{
}

OnlineDataSource::ParsingBuffer::~ParsingBuffer()
{
	clear();
}

void OnlineDataSource::ParsingBuffer::clear()
{
	framesQuantity = 0;
	DELETE_ARRAY_IF_NOT_NULL(rupFramesHeaders);
	DELETE_ARRAY_IF_NOT_NULL(rupFramesData);
	frame0ServerTime = 0;
	isSimPacket = false;
	readyToParsing = false;
}

void OnlineDataSource::ParsingBuffer::allocate(int frmsCount)
{
	Q_ASSERT(frmsCount > 0 && frmsCount <= Rup::MAX_FRAME_COUNT);

	clear();

	framesQuantity = static_cast<quint16>(frmsCount);

	rupFramesHeaders = new Rup::Header[framesQuantity];
	rupFramesData = new Rup::Data[framesQuantity];

	prepareToWriting();
}

bool OnlineDataSource::ParsingBuffer::copyRupFrame(int frameNo, qint64 serverTime,
												   bool simFrame, const Rup::Frame& rupFrame)
{
	Q_ASSERT(readyToParsing == false);

	// frameNo already checked!

	Q_ASSERT(sizeof(rupFramesHeaders[0]) == sizeof(rupFrame.header));

	// rupFrame.header already reversed!
	//
	memcpy(&rupFramesHeaders[frameNo], &rupFrame.header, sizeof(rupFrame.header));

	Q_ASSERT(sizeof(rupFramesData[0]) == sizeof(rupFrame.data));

	memcpy(&rupFramesData[frameNo], &rupFrame.data, sizeof(rupFrame.data));

	if (frameNo == 0)
	{
		if (serverTime <= frame0ServerTime)
		{
			serverTime = frame0ServerTime + 1;
		}

		frame0ServerTime = serverTime;

		isSimPacket = simFrame;
	}
	else
	{
		isSimPacket |= simFrame;
	}

	bool dataReadyToParsing = true;

	if (framesQuantity > 1)
	{
		quint16 numerator = rupFramesHeaders[frameNo].numerator;

		for(int i = 0; i < framesQuantity; i++)
		{
			if (rupFramesHeaders[i].frameSize == 0 ||
				rupFramesHeaders[i].numerator != numerator)
			{
				dataReadyToParsing = false;
				break;
			}
		}
	}

	return dataReadyToParsing;
}

void OnlineDataSource::ParsingBuffer::prepareToWriting()
{
	for(int i = 0; i < framesQuantity; i++)
	{
		rupFramesHeaders[i].frameSize = 0;
	}

	readyToParsing = false;
}

const Rup::Header& OnlineDataSource::ParsingBuffer::frame0Header() const
{
	Q_ASSERT(rupFramesHeaders != nullptr);

	return rupFramesHeaders[0];
}

const char* OnlineDataSource::ParsingBuffer::rupData() const
{
	Q_ASSERT(rupFramesData != nullptr);

	return reinterpret_cast<const char*>(rupFramesData);
}

int OnlineDataSource::ParsingBuffer::rupDataSize() const
{
	Q_ASSERT(framesQuantity != 0);

	return framesQuantity * sizeof(Rup::Data);
}

// -------------------------------------------------------------------------------
//
// OnlineDataSources class implementation
//
// -------------------------------------------------------------------------------

OnlineDataSources::OnlineDataSources()
{
}

OnlineDataSources::~OnlineDataSources()
{
	clear();
}

bool OnlineDataSources::append(OnlineDataSource* onlineSource,
							   E::LanControllerType lanType,
							   CircularLoggerShared logger)
{
	TEST_PTR_RETURN_FALSE(onlineSource);

	bool result = true;

	for(const LanControllerInfo& lci : onlineSource->lanControllersInfo()())
	{
		if (lci.isEnabled(lanType) == false ||
			lci.appDataFramesQuantity == 0)
		{
			continue;
		}

		if (m_lanControllerToSource.contains(lci.equipmentID) == true)
		{
			DEBUG_LOG_ERR(logger, QString("Duplicate OnlineDataSource adapter EquipmentID %1 (LAN type %2)").
											arg(lci.equipmentID).arg(E::valueToString(lanType)));
			result = false;
			continue;
		}

		if (m_ipToSource.contains(lci.appDataIP32()) == true)
		{
			DEBUG_LOG_ERR(logger, QString("Duplicate OnlineDataSource IP-address %1 (LAN type %2)").
										arg(lci.appDataIP).arg(E::valueToString(lanType)));
			continue;
		}

		m_sources.push_back(onlineSource);

		m_moduleToSource.insert({onlineSource->moduleEquipmentID(), onlineSource});

		const QStringList& sourceSignals = onlineSource->associatedSignals(lanType);

		for(const QString& signalID : sourceSignals)
		{
			Hash signalHash = calcHash(signalID);

			Q_ASSERT(m_signalToSource.contains(signalHash) == false);

			m_signalToSource.insert({signalHash, onlineSource});
		}

		m_lanControllerToSource.insert({lci.equipmentID, onlineSource});
		m_ipToSource.insert({lci.appDataIP32(), onlineSource});
	}

	return result;
}

void OnlineDataSources::clear()
{
	m_lanControllerToSource.clear();
	m_ipToSource.clear();
	m_signalToSource.clear();
	m_moduleToSource.clear();
	m_sources.clear();
}

bool OnlineDataSources::pushRupFrame(quint32 sourceIP,
									 qint64 serverTime,
									 bool isSimFrame,
									 Rup::Frame& rupFrame,
									 const QThread* thread)
{
	auto it = m_ipToSource.find(sourceIP);

	if (it == m_ipToSource.end())
	{
		return false;
	}

	OnlineDataSource* source = it->second;


	bool readyToParsing = source->pushRupFrame(sourceIP, serverTime, isSimFrame, rupFrame, thread);

	if (readyToParsing == true)
	{
		processingRequired(source, true);
	}

	return true;
}

void OnlineDataSources::updateDataSourcesStatistics500ms(bool oneSecond)
{
	for(OnlineDataSource* source : m_sources)
	{
		TEST_PTR_CONTINUE(source);

		bool invalidateSignals = source->updateStatistics_500ms(oneSecond);

		if (invalidateSignals == true)
		{
			processingRequired(source, false);
		}
	}
}

OnlineDataSource* OnlineDataSources::getSourceByIP(quint32 ip)
{
	auto it = m_ipToSource.find(ip);

	if (it == m_ipToSource.end())
	{
		return nullptr;
	}

	return it->second;
}

OnlineDataSource* OnlineDataSources::getSignalSource(const QString& signalID)
{
	return getSignalSource(calcHash(signalID));
}

OnlineDataSource* OnlineDataSources::getSignalSource(Hash signalHash)
{
	auto it = m_signalToSource.find(signalHash);

	if (it == m_signalToSource.end())
	{
		return nullptr;
	}

	return it->second;
}

std::vector<OnlineDataSource*>::iterator OnlineDataSources::begin()
{
	return m_sources.begin();
}

std::vector<OnlineDataSource*>::const_iterator OnlineDataSources::begin() const
{
	return m_sources.begin();
}

std::vector<OnlineDataSource*>::iterator OnlineDataSources::end()
{
	return m_sources.end();
}

std::vector<OnlineDataSource*>::const_iterator OnlineDataSources::end() const
{
	return m_sources.end();
}

void OnlineDataSources::processingRequired(OnlineDataSource* source, bool parse)
{
	TEST_PTR_RETURN(source);

	// parse == true - signal states parsing required
	// parse == false - signal states invalidation required

	std::lock_guard lg(m_processigRequiredMutex);
	m_processingRequired.emplace(source, parse);
	m_processingRequiredCondition.notify_one();
}

