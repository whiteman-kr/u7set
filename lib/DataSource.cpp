#include "DataSource.h"
#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/Crc.h"

// -----------------------------------------------------------------------------
//
// DataSource class implementation
//
// -----------------------------------------------------------------------------

QStringList DataSource::m_emptyList;

DataSource::DataSource()
{
}

DataSource::~DataSource()
{
}

int DataSource::appDataFramesQuantity() const
{
	const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::AppData);

	if (lci.isValid() == true)
	{
		return lci.appDataFramesQuantity;
	}

	return 0;
}

int DataSource::diagDataFramesQuantity() const
{
	const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::DiagData);

	if (lci.isValid() == true)
	{
		return lci.diagDataFramesQuantity;
	}

	return 0;
}

int DataSource::appDataSizeBytes() const
{
	const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::AppData);

	if (lci.isValid() == true)
	{
		return lci.appDataSizeBytes;
	}

	return 0;
}

int DataSource::diagDataSizeBytes() const
{
	const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::DiagData);

	if (lci.isValid() == true)
	{
		return lci.diagDataFramesQuantity;
	}

	return 0;
}

int DataSource::overrideAppDataWordCount() const
{
	const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::AppData);

	if (lci.isValid() == true)
	{
		return lci.overrideAppDataWordCount;
	}

	return 0;
}

int DataSource::overrideDiagDataWordCount() const
{
	const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::DiagData);

	if (lci.isValid() == true)
	{
		return lci.overrideDiagDataWordCount;
	}

	return 0;
}

quint32 DataSource::appDataUID() const
{
	const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::AppData);

	if (lci.isValid() == true)
	{
		return lci.appDataUID;
	}

	return 0;
}

quint32 DataSource::diagDataUID() const
{
	const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::DiagData);

	if (lci.isValid() == true)
	{
		return lci.diagDataUID;
	}

	return 0;
}

quint64 DataSource::tuningDataUID() const
{
	const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::Tuning);

	if (lci.isValid() == true)
	{
		return lci.tuningDataUID;
	}

	return 0;
}

void DataSource::appendAssociatedSignal(E::LanControllerType lanType, const QString& signalID)
{
	bool app = false;
	bool tun = false;

	if (LanControllerInfo::isProvideAppData(lanType) == true)
	{
		m_appSignals.append(signalID);
		app = true;
	}

	if (LanControllerInfo::isProvideTuning(lanType) == true)
	{
		m_tuningSignals.append(signalID);
		tun = true;
	}

	if (LanControllerInfo::isProvideDiagData(lanType) == true)
	{
		if (app == true || tun == true)
		{
			Q_ASSERT(false);				// diag signal can't be app or tuning signal simultaneously
		}

		m_diagSignals.append(signalID);
	}
}

void DataSource::clearAssociatedSignals(E::LanControllerType lanType)
{
	if (LanControllerInfo::isProvideAppData(lanType) == true)
	{
		m_appSignals.clear();
	}

	if (LanControllerInfo::isProvideTuning(lanType) == true)
	{
		m_tuningSignals.clear();
	}

	if (LanControllerInfo::isProvideDiagData(lanType) == true)
	{
		m_diagSignals.clear();
	}
}

const QStringList& DataSource::associatedSignals(E::LanControllerType lanType) const
{
	if (LanControllerInfo::isProvideAppData(lanType) == true)
	{
		return m_appSignals;
	}

	if (LanControllerInfo::isProvideTuning(lanType) == true)
	{
		return m_tuningSignals;
	}

	if (LanControllerInfo::isProvideDiagData(lanType) == true)
	{
		return m_diagSignals;
	}

	Q_ASSERT(false);

	return m_emptyList;
}

/*
HostAddressPort DataSource::lanHostAddressPort() const
{
	QString ip;
	int port = 0;

	int provideCount = 0;

	if (m_lanControllersInfo.isProvideTuning() == true)
	{
		ip = m_lanControllersInfo.tuningIP;
		port = m_lanControllersInfo.tuningPort;
		provideCount++;
	}

	if (m_lanControllersInfo.isProvideAppData() == true)
	{
		ip = m_lanControllersInfo.appDataIP;
		port = m_lanControllersInfo.appDataPort;
		provideCount++;
	}

	if (m_lanControllersInfo.isProvideDiagData() == true)
	{
		ip = m_lanControllersInfo.diagDataIP;
		port = m_lanControllersInfo.diagDataPort;
		provideCount++;
	}

	Q_ASSERT(provideCount == 1);

	return HostAddressPort(ip, port);
}*/

void DataSource::writeToXml(XmlWriteHelper& xml) const
{
	xml.writeStartElement(XmlElement::DATA_SOURCE);

	xml.writeStringAttribute(XmlAttribute::MODULE_EQUIPMENT_ID, m_moduleEquipmentID);
	xml.writeStringAttribute(XmlAttribute::PROFILE, m_profile);
	xml.writeStringAttribute(XmlAttribute::MODULE_PRESET_NAME, m_modulePresetName);
	xml.writeIntAttribute(XmlAttribute::MODULE_TYPE, m_moduleType, true);
	xml.writeStringAttribute(XmlAttribute::SUBSYSTEM_ID, m_subsystemID);
	xml.writeIntAttribute(XmlAttribute::SUBSYSTEM_KEY, m_subsystemKey);
	xml.writeIntAttribute(XmlAttribute::LM_NUMBER, m_lmNumber);
	xml.writeStringAttribute(XmlAttribute::SUBSYSTEM_CHANNEL, m_subsystemChannel);
	xml.writeStringAttribute(XmlAttribute::CAPTION, m_moduleCaption);
	xml.writeUInt64Attribute(XmlAttribute::MODULE_UNIQUE_ID, m_moduleUniqueID, true);

	xml.writeIntAttribute(EquipmentPropNames::APP_DATA_SIZE_BYTES, appDataSizeBytes());
	xml.writeUInt32Attribute(EquipmentPropNames::APP_DATA_UID, appDataUID(), false);
	xml.writeUInt32Attribute(EquipmentPropNames::HEX_APP_DATA_UID, appDataUID(), true);
	xml.writeIntAttribute(EquipmentPropNames::APP_DATA_FRAMES_QUANTITY, appDataFramesQuantity());
	xml.writeIntAttribute(EquipmentPropNames::OVERRIDE_APP_DATA_WORD_COUNT, overrideAppDataWordCount());

	xml.writeUInt64Attribute(EquipmentPropNames::TUNING_DATA_UID, tuningDataUID(), true);

	xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_SIZE_BYTES, diagDataSizeBytes());
	xml.writeUInt32Attribute(EquipmentPropNames::DIAG_DATA_UID, diagDataUID(), false);
	xml.writeUInt32Attribute(EquipmentPropNames::HEX_DIAG_DATA_UID, diagDataUID(), true);
	xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_FRAMES_QUANTITY, diagDataFramesQuantity());
	xml.writeIntAttribute(EquipmentPropNames::OVERRIDE_DIAG_DATA_WORD_COUNT, overrideDiagDataWordCount());

	m_lanControllersInfo.writeToXml(xml);

	xml.writeStringElement(XmlElement::APP_SIGNALS, m_appSignals.join(Separator::COMMA));
	xml.writeStringElement(XmlElement::TUNING_SIGNALS, m_tuningSignals.join(Separator::COMMA));
	xml.writeStringElement(XmlElement::DIAG_SIGNALS, m_diagSignals.join(Separator::COMMA));

	writeAdditionalSectionsToXml(xml);

	xml.writeEndElement();	// </DataSource>
}

bool DataSource::readFromXml(XmlReadHelper& xml)
{
	if (xml.findElement(XmlElement::DATA_SOURCE) == false)
	{
		return false;
	}

	bool result = true;

	result &= xml.readStringAttribute(XmlAttribute::MODULE_EQUIPMENT_ID, &m_moduleEquipmentID);
	result &= xml.readStringAttribute(XmlAttribute::PROFILE, &m_profile);
	result &= xml.readStringAttribute(XmlAttribute::MODULE_PRESET_NAME, &m_modulePresetName);
	result &= xml.readIntAttribute(XmlAttribute::MODULE_TYPE, &m_moduleType);
	result &= xml.readStringAttribute(XmlAttribute::SUBSYSTEM_ID,&m_subsystemID);
	result &= xml.readIntAttribute(XmlAttribute::SUBSYSTEM_KEY, &m_subsystemKey);
	result &= xml.readIntAttribute(XmlAttribute::LM_NUMBER, &m_lmNumber);
	result &= xml.readStringAttribute(XmlAttribute::SUBSYSTEM_CHANNEL,&m_subsystemChannel);
	result &= xml.readStringAttribute(XmlAttribute::CAPTION, &m_moduleCaption);
	result &= xml.readUInt64Attribute(XmlAttribute::MODULE_UNIQUE_ID, &m_moduleUniqueID);

	if (xml.findElement(XmlElement::LAN_CONTROLLERS) == false)
	{
		return false;
	}
	result &= m_lanControllersInfo.readFromXml(xml);

	QString signalsStr;

	result &= xml.readStringElement(XmlElement::APP_SIGNALS, &signalsStr, true);

	m_appSignals = signalsStr.split(Separator::COMMA, Qt::SkipEmptyParts);

	result &= xml.readStringElement(XmlElement::TUNING_SIGNALS, &signalsStr, true);

	m_tuningSignals = signalsStr.split(Separator::COMMA, Qt::SkipEmptyParts);

	result &= xml.readStringElement(XmlElement::DIAG_SIGNALS, &signalsStr, true);

	m_diagSignals = signalsStr.split(Separator::COMMA, Qt::SkipEmptyParts);

	result &= readAdditionalSectionsFromXml(xml);

	RETURN_IF_FALSE(result);

	m_id = generateID();

	return result;
}

void DataSource::writeAdditionalSectionsToXml(XmlWriteHelper&) const
{
}

bool DataSource::readAdditionalSectionsFromXml(XmlReadHelper&)
{
	return true;
}

bool DataSource::saveToProto(Network::DataSourceInfo* proto) const
{
	if (proto == nullptr)
	{
		assert(false);
		return false;
	}

	proto->set_id(m_id);
	proto->set_moduleequipmentid(m_moduleEquipmentID.toStdString());
	proto->set_profile(m_profile.toStdString());
	proto->set_modulepresetname(m_modulePresetName.toStdString());
	proto->set_moduletype(m_moduleType);
	proto->set_modulecaption(m_moduleCaption.toStdString());
	proto->set_moduleuniqueid(m_moduleUniqueID);
	proto->set_subsystemid(m_subsystemID.toStdString());
	proto->set_subsystemkey(m_subsystemKey);
	proto->set_lmnumber(m_lmNumber);
	proto->set_subsystemchannel(m_subsystemChannel.toStdString());

	proto->clear_lancontrollerinfo();

	for(const LanControllerInfo& lci : m_lanControllersInfo())
	{
		Network::LanControllerInfo* protoLci = proto->add_lancontrollerinfo();
		lci.saveToProto(protoLci);
	}

	return true;
}

bool DataSource::loadFromProto(const Network::DataSourceInfo& proto)
{
	m_id = proto.id();
	m_moduleEquipmentID = QString::fromStdString(proto.moduleequipmentid());
	m_profile = QString::fromStdString(proto.profile());
	m_modulePresetName = QString::fromStdString(proto.modulepresetname());
	m_moduleType = proto.moduletype();
	m_moduleCaption = QString::fromStdString(proto.modulecaption());
	m_moduleUniqueID = proto.moduleuniqueid();
	m_subsystemID = QString::fromStdString(proto.subsystemid());
	m_subsystemKey = proto.subsystemkey();
	m_lmNumber = proto.lmnumber();
	m_subsystemChannel = QString::fromStdString(proto.subsystemchannel());

	m_lanControllersInfo.clear();

	int count = proto.lancontrollerinfo_size();

	for(int i = 0; i < count; i++)
	{
		LanControllerInfo lci;
		lci.loadFromProto(proto.lancontrollerinfo(i));

		m_lanControllersInfo.append(lci);
	}

	return true;
}

quint64 DataSource::generateID() const
{
	if (m_lanControllersInfo().size() == 0)
	{
		Q_ASSERT(false);
		return 0;
	}

	Crc64 crc;

	for(const LanControllerInfo& lci :  m_lanControllersInfo())
	{
		crc.add(lci.equipmentID);
		crc.add(TO_INT(lci.lanControllerType));

		crc.add(lci.tuningIP);
		crc.add(lci.tuningPort);

		crc.add(lci.appDataIP);
		crc.add(lci.appDataPort);

		crc.add(lci.diagDataIP);
		crc.add(lci.diagDataPort);
	}

	return crc.result();
}


// -----------------------------------------------------------------------------
//
// DataSourceOnline class implementation
//
// -----------------------------------------------------------------------------

const QString DataSourceOnline::DATE_TIME_FORMAT_STR("%1:%2:%3.%4 %5/%6/%7");

DataSourceOnline::DataSourceOnline() :
	m_rupFrameTimeQueue(10)
{
}

DataSourceOnline::~DataSourceOnline()
{
	if (m_rupFramesHeaders != nullptr)
	{
		delete [] m_rupFramesHeaders;
	}

	if (m_rupFramesData != nullptr)
	{
		delete [] m_rupFramesData;
	}
}

bool DataSourceOnline::initQueue()
{
	int queueSize = FastThreadSafeQueue<RupFrameTime>::MIN_QUEUE_SIZE;

	if (appDataFramesQuantity() > 0)
	{
		queueSize = appDataFramesQuantity() * 200 * 3;	// 3 seconds queue;
	}

	m_rupFrameTimeQueue.resize(queueSize);

	setRupFramesQueueSize(queueSize);

	return true;
}

void DataSourceOnline::updateUptime()
{
	if (m_firstPacketSystemTime == 0 || m_lastPacketSystemTime == 0)
	{
		m_uptime = 0;
	}
	else
	{
		m_uptime = (m_lastPacketSystemTime - m_firstPacketSystemTime) / 1000;
	}
}

QString DataSourceOnline::rupFramePlantTimeStr() const
{
	return getTimeStr(m_rupFramePlantTime);
}

QString DataSourceOnline::lastPacketSystemTimeStr() const
{
	return getTimeStr(m_lastPacketSystemTime);
}

void DataSourceOnline::pushRupFrame(quint32 sourceIP,
									qint64 serverTime,
									bool isSimFrame,
									const Rup::Frame& rupFrame,
									const QThread* thread)
{
	RupFrameTime* rupFrameTime = m_rupFrameTimeQueue.beginPush(thread);

	if (rupFrameTime != nullptr)
	{
		rupFrameTime->sourceIP = sourceIP;
		rupFrameTime->serverTime = serverTime;
		rupFrameTime->isSimFrame = isSimFrame;
		memcpy(&rupFrameTime->rupFrame, &rupFrame, sizeof(rupFrame));
	}
	else
	{
		// is not an error - queue is full
	}

	m_rupFrameTimeQueue.completePush(thread, &m_rupFramesQueueCurSize, &m_rupFramesQueueCurMaxSize);
}

bool DataSourceOnline::takeProcessingOwnership(const QThread* processingThread)
{
	const QThread* expected = nullptr;

	bool result = m_processingOwner.compare_exchange_strong(expected,  processingThread);

	// if ownership has been taken by processingWorker - function returns TRUE
	//
	// result == FALSE is Ok, means that another thread is already take ownership

	return result;
}

bool DataSourceOnline::releaseProcessingOwnership(const QThread* processingThread)
{
	bool result = m_processingOwner.compare_exchange_strong(processingThread,  nullptr);

	assert(result == true);	// releaseProcessingOwnership must be called by processingWorker == m_processingOwner only !!!

	return result;
}

bool DataSourceOnline::processRupFrameTimeQueue(const QThread* thread)
{
	int count = 0;

	m_dataReadyToParsing = false;

	do
	{
		RupFrameTime* rupFrameTime = m_rupFrameTimeQueue.beginPop(thread);

		if (rupFrameTime == nullptr)
		{
			if (m_dataReceives == true)
			{
				// check m_lastPacketSystemTime
				//
				qint64 now = QDateTime::currentMSecsSinceEpoch();

				if (now - m_lastPacketSystemTime > APP_DATA_SOURCE_TIMEOUT)
				{
					m_rupDataTimes = m_lastRupDataTimes;
					m_rupDataTimes += APP_DATA_SOURCE_TIMEOUT;

					m_state = E::DataSourceState::NoData;

					m_dataRecevingTimeout = true;
					m_dataReceives = false;
					m_dataReadyToParsing = true;
					m_firstRupFrame = true;

					m_firstPacketSystemTime = 0;
					m_lastPacketSystemTime = 0;

					m_dataReceivingRate = 0;

					m_prevCalcTime = -1;

					updateUptime();
				}
			}

			break;			// has no frames to processing, exit from processRupFrameTimeQueue, return FALSE
							// m_rupFrameTimeQueue.completePop is not required !!!
		}

		m_lastPacketSystemTime = rupFrameTime->serverTime;

		if (m_firstPacketSystemTime == 0)
		{
			m_firstPacketSystemTime = m_lastPacketSystemTime;
		}

		updateUptime();

		if (m_state == E::DataSourceState::NoData)
		{
			m_receivedDataSize = 0;
			m_receivedFramesCount = 0;
			m_receivedPacketCount = 0;
			m_lostPacketCount = 0;
			m_processedPacketCount = 0;

			m_errorProtocolVersion = 0;
			m_errorFramesQuantity = 0;
			m_errorFrameNo = 0;
			m_errorDataID = 0;
			m_errorFrameSize = 0;
			m_errorDuplicatePlantTime = 0;
			m_errorNonmonotonicPlantTime = 0;
			m_errorPlantTimeFormat = 0;
		}

		m_state = E::DataSourceState::ReceiveData;

		m_dataRecevingTimeout = false;
		m_dataReceives = true;
		m_receivedDataSize += sizeof(Rup::Frame);

		calcDataReceivingRate();

		if (m_dataProcessingEnabled == false)
		{
			break;
		}

		do
		{
			Rup::Header& rupFrameHeader = rupFrameTime->rupFrame.header;

			rupFrameHeader.reverseBytes();

			// rupFrame's protocol version checking
			//
			if (rupFrameHeader.protocolVersion != 5)
			{
				m_errorProtocolVersion++;
				break;
			}

			// rupFrame's data ID checking
			//
			m_receivedDataID = rupFrameHeader.dataId;

			if (m_receivedDataID != appDataUID())
			{
				m_errorDataID++;

				if (m_errorDataID > 0 && (m_errorDataID % 500) == 0)
				{
					QString msg = QString("Wrong DataID from %1 (0x%2, waiting 0x%3), packet processing skiped").
							arg(rupFrameTime->sourceIP).
							arg(QString::number(rupFrameHeader.dataId, 16)).
							arg(QString::number(appDataUID(), 16));

					qDebug() << C_STR(msg);
				}

				break;
			}

			// RupFrame's framesQuantity and frameNo checkng
			//
			if (rupFrameHeader.framesQuantity > Rup::MAX_FRAME_COUNT)
			{
				m_errorFramesQuantity++;
				break;
			}

			if (rupFrameHeader.frameNumber >= rupFrameHeader.framesQuantity)
			{
				m_errorFrameNo++;
				break;
			}

			// collect rupFrames
			//
			m_dataReadyToParsing = collect(*rupFrameTime);

			if (m_dataReadyToParsing == true)
			{
				m_rupFramePlantTime = m_rupDataTimes.plant.timeStamp;
				m_processedPacketCount++;

				// rupFrame's numerator tracking
				//
				quint16 numerator = rupFrameHeader.numerator;

				if (m_firstRupFrame == true)
				{
					m_rupFrameNumerator = numerator;
					m_firstRupFrame = false;
				}
				else
				{
					if (m_rupFrameNumerator != numerator)
					{
						if (m_rupFrameNumerator < numerator)
						{
							m_lostPacketCount += numerator - m_rupFrameNumerator;
						}
						else
						{
							m_lostPacketCount += 0xFFFF - m_rupFrameNumerator + numerator;
						}

						m_rupFrameNumerator = numerator;
					}
				}

				m_rupFrameNumerator++;
			}

			break;
		}
		while(true);

		m_rupFrameTimeQueue.completePop(thread);

		count++;
	}
	while(m_dataReadyToParsing == false && count < 100);

	return m_dataReadyToParsing;
}

bool DataSourceOnline::getDataToParsing(Times* times,
										bool* isSimPacket,
										quint16* packetNo,
										const char** rupData,
										int* rupDataSize,
										bool* dataReceivingTimeout)
{
	if (m_dataReadyToParsing == false)
	{
		assert(false);
		return false;
	}

#ifdef QT_DEBUG

	if (times == nullptr || packetNo == nullptr || rupData == nullptr || rupDataSize == nullptr || dataReceivingTimeout == nullptr)
	{
		assert(false);
		return false;
	}

#endif

	*times = m_rupDataTimes;
	*isSimPacket = m_isSimPacket;
	*packetNo = m_packetNo;
	*rupData = reinterpret_cast<const char*>(m_rupFramesData);
	*rupDataSize = m_rupDataSize;
	*dataReceivingTimeout = m_dataRecevingTimeout;

	m_dataReadyToParsing = false;

	return true;
}

bool DataSourceOnline::collect(const RupFrameTime& rupFrameTime)
{
	// rupFrameTime.rupFrame.header already reverseByted !
	//
	const Rup::Header& rupFrameHeader = rupFrameTime.rupFrame.header;

	quint32 framesQuantity = rupFrameHeader.framesQuantity;

	if (framesQuantity > m_framesQuantityAllocated)
	{
		if (reallocate(framesQuantity) == false)
		{
			return false;
		}
	}

	quint32 frameNumber = rupFrameHeader.frameNumber;

	if (frameNumber >= framesQuantity)
	{
		m_errorFrameNo++;
		return false;
	}

	if (frameNumber >= m_framesQuantityAllocated)
	{
		assert(false);
		return false;
	}

	if (frameNumber == 0)
	{
		m_frame0ServerTime = rupFrameTime.serverTime;
		m_isSimPacket = rupFrameTime.isSimFrame;
	}
	else
	{
		m_isSimPacket |= rupFrameTime.isSimFrame;
	}

	// copy RUP frame header
	//
	memcpy(m_rupFramesHeaders + frameNumber, &rupFrameHeader, sizeof(rupFrameHeader));

	// copy RUP frame data
	//
	memcpy(m_rupFramesData + frameNumber, &rupFrameTime.rupFrame.data, sizeof(rupFrameTime.rupFrame.data));

	m_receivedFramesCount++;

	// check packet parts
	//
	bool dataReady = true;

	quint16 numerator0 = m_rupFramesHeaders[0].numerator;

	for(quint32 i = 1; i < framesQuantity; i++)
	{
		dataReady &= m_rupFramesHeaders[i].numerator == numerator0;
	}

	if (dataReady == false)
	{
		return false;
	}

	m_packetNo = numerator0;

	m_receivedPacketCount++;

	const Rup::TimeStamp& timeStamp = m_rupFramesHeaders[0].timeStamp;

	if (timeStamp.year < 2000 || timeStamp.year > 2500 ||
		timeStamp.month < 1 || timeStamp.month > 12 ||
		timeStamp.day < 1 || timeStamp.day > 31 ||
		timeStamp.hour > 23 || timeStamp.minute > 59 ||
		timeStamp.second > 59 || timeStamp.millisecond > 999)
	{
		m_errorPlantTimeFormat++;

		if (m_timeErrLog != nullptr)
		{
			DEBUG_LOG_ERR(m_timeErrLog, QString("Source %1 time format error %2").
											arg(moduleEquipmentID()).
											arg(getTimeStr(timeStamp)));
		}
	}

	QDateTime plantTime;

	// don't delete this to prevent plantTime conversion from Local to UTC time during call plantTime.toMSecsSinceEpoch()!!!
	//
	plantTime.setTimeSpec(Qt::UTC);

	plantTime.setDate(QDate(timeStamp.year, timeStamp.month, timeStamp.day));
	plantTime.setTime(QTime(timeStamp.hour, timeStamp.minute, timeStamp.second, timeStamp.millisecond));

	m_rupDataTimes.plant.timeStamp = plantTime.toMSecsSinceEpoch();
	m_rupDataTimes.system.timeStamp = m_frame0ServerTime;

	QDateTime localTime = QDateTime::fromMSecsSinceEpoch(m_frame0ServerTime);

	// don't delete this to prevent localTime conversion from Local to UTC time during call localTime.toMSecsSinceEpoch()!!!
	//
	localTime.setTimeSpec(Qt::UTC);

	m_rupDataTimes.local.timeStamp = localTime.toMSecsSinceEpoch();

	//

	m_rupDataSize = framesQuantity * sizeof(Rup::Data);

	if (m_lastRupDataTimes.plant.timeStamp == m_rupDataTimes.plant.timeStamp)
	{
		m_errorDuplicatePlantTime++;

		if (m_timeErrLog != nullptr)
		{
			DEBUG_LOG_ERR(m_timeErrLog, QString("Source %1 duplicate time %2").
											arg(moduleEquipmentID()).
											arg(getTimeStr(timeStamp)));
		}
	}

	if (m_lastRupDataTimes.plant.timeStamp > m_rupDataTimes.plant.timeStamp)
	{
		m_errorNonmonotonicPlantTime++;

		if (m_timeErrLog != nullptr)
		{
			DEBUG_LOG_ERR(m_timeErrLog, QString("Source %1 non monotonic time %2 (prev time %3)").
											arg(moduleEquipmentID()).
											arg(getTimeStr(timeStamp)).
											arg(getTimeStr(m_lastRupDataTimes.plant.timeStamp)));
		}
	}

	m_lastRupDataTimes = m_rupDataTimes;

	return true;
}

bool DataSourceOnline::reallocate(quint32 framesQuantity)
{
	m_dataReadyToParsing = false;					// !!!  m_rupFramesData will be freed

	if (m_rupFramesHeaders != nullptr)
	{
		delete [] m_rupFramesHeaders;
		m_rupFramesHeaders = nullptr;
	}

	if (m_rupFramesData != nullptr)
	{
		delete [] m_rupFramesData;
		m_rupFramesData = nullptr;
	}

	m_framesQuantityAllocated = 0;

	if (framesQuantity < 1 || framesQuantity > Rup::MAX_FRAME_COUNT)
	{
		assert(false);
		return false;
	}

	m_rupFramesHeaders = new Rup::Header[framesQuantity];
	m_rupFramesData = new Rup::Data[framesQuantity];

	m_framesQuantityAllocated = framesQuantity;

	return true;
}

void DataSourceOnline::calcDataReceivingRate()
{
	if (m_prevCalcTime == -1)
	{
		m_prevCalcTime = QDateTime::currentMSecsSinceEpoch();
		m_prevReceivedSize = m_receivedDataSize;
		m_firstCalc = true;
		return;
	}

	m_calcFramesCtr++;

	if (m_calcFramesCtr < 100)
	{
		return;
	}

	m_calcFramesCtr = 0;

	qint64 now = QDateTime::currentMSecsSinceEpoch();

	qint64 dT = now - m_prevCalcTime;

	if (m_firstCalc == false)
	{
		if (dT < DATA_RECEIVING_RATE_CALC_PERIOD)
		{
			return;
		}
	}

	m_firstCalc = false;

	m_dataReceivingRate = static_cast<double>(m_receivedDataSize - m_prevReceivedSize) / (dT / 1000.0);		// Bytes per second

	m_prevCalcTime = now;
	m_prevReceivedSize = m_receivedDataSize;

	return;
}

QString DataSourceOnline::getTimeStr(qint64 timeMs) const
{
	QDateTime dt = QDateTime::fromMSecsSinceEpoch(timeMs, Qt::UTC, 0);

	QDate date = dt.date();
	QTime time = dt.time();

	return 	QString(DATE_TIME_FORMAT_STR).
				arg(time.hour(), 2, 10, QLatin1Char('0')).
				arg(time.minute(), 2, 10, QLatin1Char('0')).
				arg(time.second(), 2, 10, QLatin1Char('0')).
				arg(time.msec(), 3, 10, QLatin1Char('0')).
				arg(date.day(), 2, 10, QLatin1Char('0')).
				arg(date.month(), 2, 10, QLatin1Char('0')).
				arg(date.year(), 4, 10, QLatin1Char('0'));
}

QString DataSourceOnline::getTimeStr(const Rup::TimeStamp& ts) const
{
	return 	QString(DATE_TIME_FORMAT_STR).
				arg(ts.hour, 2, 10, QLatin1Char('0')).
				arg(ts.minute, 2, 10, QLatin1Char('0')).
				arg(ts.second, 2, 10, QLatin1Char('0')).
				arg(ts.millisecond, 3, 10, QLatin1Char('0')).
				arg(ts.day, 2, 10, QLatin1Char('0')).
				arg(ts.month, 2, 10, QLatin1Char('0')).
				arg(ts.year, 4, 10, QLatin1Char('0'));
}



