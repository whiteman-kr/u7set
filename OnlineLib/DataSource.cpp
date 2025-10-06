#include "DataSource.h"
#include "../UtilsLib/Crc.h"
#include "../UtilsLib/WUtils.h"

namespace OnlineLib
{
	// -----------------------------------------------------------------------------
	//
	// DataSource class implementation
	//
	// -----------------------------------------------------------------------------

	QStringList DataSource::m_emptyList;

	DataSource::DataSource() {}

	DataSource::~DataSource() {}

	quint32 DataSource::rupAppDataUID() const
	{
		const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::AppData);

		if (lci.isValid() == true)
		{
			return lci.rupAppDataUID;
		}

		return 0;
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

	int DataSource::appDataSizeBytes() const
	{
		const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::AppData);

		if (lci.isValid() == true)
		{
			return lci.appDataSizeBytes;
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

	int DataSource::appSignalsCount() const
	{
		return TO_INT(m_appSignals.size());
	}

	quint32 DataSource::rupDiagDataUID() const
	{
		const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::DiagData);

		if (lci.isValid() == true)
		{
			return lci.rupDiagDataUID;
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

	int DataSource::diagDataSizeBytes() const
	{
		const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::DiagData);

		if (lci.isValid() == true)
		{
			return lci.diagDataFramesQuantity;
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

	int DataSource::diagSignalsCount() const
	{
		return TO_INT(m_diagSignals.size());
	}

	quint32 DataSource::rupTuningDataUID() const
	{
		const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::Tuning);

		if (lci.isValid() == true)
		{
			return lci.rupTuningDataUID;
		}

		return 0;
	}

	quint64 DataSource::fotipTuningDataUID() const
	{
		const LanControllerInfo& lci = m_lanControllersInfo.getFirstCompatibleController(E::LanControllerType::Tuning);

		if (lci.isValid() == true)
		{
			return lci.fotipTuningDataUID;
		}

		return 0;
	}

	int DataSource::fotipVersion() const
	{
		return m_lanControllersInfo.fotipVersion();
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
				Q_ASSERT(false); // diag signal can't be app or tuning signal simultaneously
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

	void DataSource::appendSwCalcSignal(const QString& appSignalID)
	{
		m_swCalcSignals.append(appSignalID);
	}

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
		xml.writeIntAttribute(XmlAttribute::MODULE_WORKCYCLE_MCS, m_moduleWorkcycle_mcs);

		xml.writeIntAttribute(EquipmentPropNames::APP_DATA_SIZE_BYTES, appDataSizeBytes());
		xml.writeUInt32Attribute(EquipmentPropNames::RUP_APP_DATA_UID, rupAppDataUID(), false);
		xml.writeUInt32Attribute(EquipmentPropNames::HEX_RUP_APP_DATA_UID, rupAppDataUID(), true);
		xml.writeIntAttribute(EquipmentPropNames::APP_DATA_FRAMES_QUANTITY, appDataFramesQuantity());
		xml.writeIntAttribute(EquipmentPropNames::OVERRIDE_APP_DATA_WORD_COUNT, overrideAppDataWordCount());

		xml.writeUInt32Attribute(EquipmentPropNames::RUP_TUNING_DATA_UID, rupTuningDataUID(), false);
		xml.writeUInt32Attribute(EquipmentPropNames::HEX_RUP_TUNING_DATA_UID, rupTuningDataUID(), true);

		xml.writeUInt64Attribute(EquipmentPropNames::FOTIP_TUNING_DATA_UID, fotipTuningDataUID(), false);
		xml.writeUInt32Attribute(EquipmentPropNames::HEX_FOTIP_TUNING_DATA_UID, fotipTuningDataUID(), true);

		xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_SIZE_BYTES, diagDataSizeBytes());
		xml.writeUInt32Attribute(EquipmentPropNames::RUP_DIAG_DATA_UID, rupDiagDataUID(), false);
		xml.writeUInt32Attribute(EquipmentPropNames::HEX_RUP_DIAG_DATA_UID, rupDiagDataUID(), true);
		xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_FRAMES_QUANTITY, diagDataFramesQuantity());
		xml.writeIntAttribute(EquipmentPropNames::OVERRIDE_DIAG_DATA_WORD_COUNT, overrideDiagDataWordCount());

		m_lanControllersInfo.writeToXml(xml);

		xml.writeStringElement(XmlElement::APP_SIGNALS, m_appSignals.join(Separator::COMMA));
		xml.writeStringElement(XmlElement::TUNING_SIGNALS, m_tuningSignals.join(Separator::COMMA));
		xml.writeStringElement(XmlElement::DIAG_SIGNALS, m_diagSignals.join(Separator::COMMA));
		xml.writeStringElement(XmlElement::SW_CALC_SIGNALS, m_swCalcSignals.join(Separator::COMMA));

		writeAdditionalSectionsToXml(xml);

		xml.writeEndElement(); // </DataSource>
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
		result &= xml.readStringAttribute(XmlAttribute::SUBSYSTEM_ID, &m_subsystemID);
		result &= xml.readIntAttribute(XmlAttribute::SUBSYSTEM_KEY, &m_subsystemKey);
		result &= xml.readIntAttribute(XmlAttribute::LM_NUMBER, &m_lmNumber);
		result &= xml.readStringAttribute(XmlAttribute::SUBSYSTEM_CHANNEL, &m_subsystemChannel);
		result &= xml.readStringAttribute(XmlAttribute::CAPTION, &m_moduleCaption);
		result &= xml.readUInt64Attribute(XmlAttribute::MODULE_UNIQUE_ID, &m_moduleUniqueID);
		result &= xml.readIntAttribute(XmlAttribute::MODULE_WORKCYCLE_MCS, &m_moduleWorkcycle_mcs);

		m_workcycle_ms = moduleWorkcycle_ms();

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

		result &= xml.readStringElement(XmlElement::SW_CALC_SIGNALS, &signalsStr, true);

		m_swCalcSignals = signalsStr.split(Separator::COMMA, Qt::SkipEmptyParts);

		result &= readAdditionalSectionsFromXml(xml);

		RETURN_IF_FALSE(result);

		m_id = generateID();

		return result;
	}

	void DataSource::writeAdditionalSectionsToXml(XmlWriteHelper&) const {}

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
		proto->set_modulepresetname(m_modulePresetName.toStdString());
		proto->set_moduletype(m_moduleType);
		proto->set_modulecaption(m_moduleCaption.toStdString());
		proto->set_moduleuniqueid(m_moduleUniqueID);
		proto->set_subsystemid(m_subsystemID.toStdString());
		proto->set_subsystemkey(m_subsystemKey);
		proto->set_lmnumber(m_lmNumber);
		proto->set_subsystemchannel(m_subsystemChannel.toStdString());
		proto->set_workcycle_mcs(m_moduleWorkcycle_mcs);

		proto->clear_lancontrollerinfo();

		proto->set_rupprotocolversion(m_lanControllersInfo.rupVersion());
		proto->set_fotipprotocolversion(m_lanControllersInfo.fotipVersion());

		for (const LanControllerInfo& lci : m_lanControllersInfo())
		{
			Network::LanControllerInfo* protoLci = proto->add_lancontrollerinfo();
			lci.saveToProto(protoLci);
		}

		proto->set_profile(m_profile.toStdString());
		proto->set_acquiredsignalscount(m_acquiredSignalsCount);
		proto->set_expecteddatauid(getExpectedDataUID());

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
		m_moduleWorkcycle_mcs = proto.workcycle_mcs();

		m_lanControllersInfo.clear();

		m_lanControllersInfo.setRupVersion(proto.rupprotocolversion());
		m_lanControllersInfo.setFotipVersion(proto.fotipprotocolversion());

		int count = proto.lancontrollerinfo_size();

		for (int i = 0; i < count; i++)
		{
			LanControllerInfo lci;
			lci.loadFromProto(proto.lancontrollerinfo(i));

			m_lanControllersInfo.append(lci);
		}

		return true;
	}

	QString DataSource::getTimeStr(qint64 timeMs)
	{
		QDateTime dt = QDateTime::fromMSecsSinceEpoch(timeMs, TIME_ZONE_UTC);

		QDate date = dt.date();
		QTime time = dt.time();

		return QString(FormatStr::DATE_TIME_FORMAT_STR)
			.arg(time.hour(), 2, 10, QLatin1Char('0'))
			.arg(time.minute(), 2, 10, QLatin1Char('0'))
			.arg(time.second(), 2, 10, QLatin1Char('0'))
			.arg(time.msec(), 3, 10, QLatin1Char('0'))
			.arg(date.day(), 2, 10, QLatin1Char('0'))
			.arg(date.month(), 2, 10, QLatin1Char('0'))
			.arg(date.year(), 4, 10, QLatin1Char('0'));
	}

	QString DataSource::getTimeStr(const Rup::TimeStamp& ts)
	{
		return QString(FormatStr::DATE_TIME_FORMAT_STR)
			.arg(ts.hour, 2, 10, QLatin1Char('0'))
			.arg(ts.minute, 2, 10, QLatin1Char('0'))
			.arg(ts.second, 2, 10, QLatin1Char('0'))
			.arg(ts.millisecond, 3, 10, QLatin1Char('0'))
			.arg(ts.day, 2, 10, QLatin1Char('0'))
			.arg(ts.month, 2, 10, QLatin1Char('0'))
			.arg(ts.year, 4, 10, QLatin1Char('0'));
	}

	quint64 DataSource::generateID() const
	{
		if (m_lanControllersInfo().size() == 0)
		{
			Q_ASSERT(false);
			return 0;
		}

		Crc64 crc;

		for (const LanControllerInfo& lci : m_lanControllersInfo())
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

	// const QString DataSourceOnline::DATE_TIME_FORMAT_STR("%1:%2:%3.%4 %5/%6/%7");

	DataSourceOnline::DataSourceOnline() :
		m_writeBufferIndex(PARSING_BUFFERS_COUNT),
		m_readBufferIndex(PARSING_BUFFERS_COUNT)
	{
	}

	DataSourceOnline::~DataSourceOnline()
	{
		clearParsingBuffers();
	}

	bool DataSourceOnline::initParsingBuffers(int framesQuantity)
	{
		clearParsingBuffers();

		m_parsingBuffers.reserve(PARSING_BUFFERS_COUNT);

		for (int i = 0; i < PARSING_BUFFERS_COUNT; i++)
		{
			ParsingBuffer* pb = new ParsingBuffer;
			pb->allocate(framesQuantity);

			m_parsingBuffers.push_back(pb);
		}

		return true;
	}

	void DataSourceOnline::clearParsingBuffers()
	{
		for (ParsingBuffer* pb : m_parsingBuffers)
		{
			DELETE_IF_NOT_NULL(pb);
		}

		m_parsingBuffers.clear();
	}

	void DataSourceOnline::pushRupFrame(quint32 sourceIP,
										qint64 serverTime,
										bool isSimFrame,
										Rup::Frame& rupFrame,
										quint32 expectedDataUID)
	{
		Q_UNUSED(sourceIP);

		m_receivedFramesCount++;
		m_receivedDataSize += (isSimFrame == true ? sizeof(Rup::SimFrame) : sizeof(Rup::Frame));

		if (m_dataProcessingEnabled == false)
		{
			return;
		}

		if (m_parsingBuffers[m_writeBufferIndex]->readyToParsing == true)
		{
			if (moveToNextWriteBuffer() == false)
			{
				m_lostPacketCount++;
				return;
			}
		}

		//

		rupFrame.header.reverseBytes();

		if (rupFrame.header.protocolVersion != rupVersion())
		{
			m_errorProtocolVersion++;
			return;
		}

		//

		ParsingBuffer& writeBuffer = *m_parsingBuffers[m_writeBufferIndex];

		if (rupFrame.header.framesQuantity != writeBuffer.framesQuantity)
		{
			Q_ASSERT(false);
			m_errorFramesQuantity++;
			return;
		}

		//

		int frameNo = rupFrame.header.frameNumber;

		if (frameNo >= writeBuffer.framesQuantity)
		{
			Q_ASSERT(false);
			m_errorFrameNo++;
			return;
		}

		//

		m_receivedDataID = rupFrame.header.dataId;

		if (m_receivedDataID != expectedDataUID)
		{
			m_errorDataID++;

			if ((m_errorDataID % 600) == 0)
			{
				qDebug() << C_STR(QString("%1 wrong data UID, expected 0x%2 received 0x%3")
									  .arg(moduleEquipmentID())
									  .arg(expectedDataUID, 8, 16)
									  .arg(m_receivedDataID, 8, 16));
			}
			return;
		}

		//

		bool readyToParsing = writeBuffer.copyRupFrame(frameNo, serverTime, isSimFrame, rupFrame);

		if (readyToParsing == true)
		{
			moveToNextWriteBuffer();
		}
	}

	bool DataSourceOnline::updateStatistics_500ms(int oneSecond, QString& logStr)
	{
		Q_UNUSED(logStr);

		bool invalidateSignals = false;

		qint64 now = QDateTime::currentMSecsSinceEpoch();

		if (m_lastPacketServerTime != 0 && (now - m_lastPacketServerTime > APP_DATA_SOURCE_TIMEOUT))
		{
			m_rupTimes = m_lastRupTimes;
			m_rupTimes += moduleWorkcycle_ms();

			if (m_receivesData == true)
			{
				invalidateSignals = true;

				// logStr = QString("Source %1 timeout! nowTime = %2, lastTime = %3 (diff = %4)").
				// 		 arg(moduleEquipmentID()).arg(now).arg(m_lastPacketServerTime).arg(now - m_lastPacketServerTime);
			}

			m_receivesData = false;

			clearStatistics();
		}
		else
		{
			if (oneSecond)
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

	bool DataSourceOnline::parseNextBuffer()
	{
		int ctr = 0;

		do
		{
			if (moveToNextReadBuffer() == false)
			{
				break;
			}

			parseBuffer(*m_parsingBuffers[m_readBufferIndex]);

			ctr++;
		} while (ctr < 50);

		return true;
	}

	bool DataSourceOnline::parseBuffer(ParsingBuffer& readBuffer)
	{
		Q_UNUSED(readBuffer);
		return true;
	}

	void DataSourceOnline::timeCorrection(const ParsingBuffer& readBuffer)
	{
		const Rup::Header& header = readBuffer.frame0Header();

		qint64 headerNumerator = header.numerator;

		// packet Numerator checking

		qint64 dN = 0;

		if (headerNumerator < m_rupFrameNumerator)
		{
			dN = 65536 + headerNumerator - m_rupFrameNumerator;
		}
		else
		{
			dN = headerNumerator - m_rupFrameNumerator;
		}

		m_lostPacketCount += dN - 1;

		qint64 frame0ServerTime = readBuffer.frame0ServerTime;

		qint64 dT = 0;

		dT = frame0ServerTime - m_lastPacketServerTime;

		if (m_rupFrameNumerator == -1 || m_lastPacketServerTime == 0 || dN > 10 || dT > 50)
		{
			// no time correction
			//
			m_rupFrameNumerator = headerNumerator;
			m_lastPacketServerTime = frame0ServerTime;
			return;
		}

		m_rupFrameNumerator = headerNumerator;

		// packet SystemTime checking

		static const qint64 MAX_TIME_ERROR = static_cast<qint64>(m_workcycle_ms * 0.4);

		if (dT > dN * m_workcycle_ms + MAX_TIME_ERROR)
		{
/*			if (moduleEquipmentID() == "SYSTEMID_CLIENTTEST_CH11_MD00")
			{
				logStr = QString("correction: pkt = %1, lastServTime = %2, frame0ServTime = %3, dT = %4, dN = %5, correctedTime %6").
						 arg(headerNumerator).
						 arg(m_lastPacketServerTime).
						 arg(frame0ServerTime).
						 arg(dT).
						 arg(dN).
						 arg(m_lastPacketServerTime + dN * m_workcycle_ms + 1);
			}*/

			m_lastPacketServerTime += dN * m_workcycle_ms + 1;
		}
		else
		{
			m_lastPacketServerTime = frame0ServerTime;
		}
	}

	void DataSourceOnline::checkPlantTime(const Rup::TimeStamp& plantTimeStamp)
	{
		if (plantTimeStamp.year < 2000 || plantTimeStamp.year > 2500 || plantTimeStamp.month < 1 || plantTimeStamp.month > 12 ||
			plantTimeStamp.day < 1 || plantTimeStamp.day > 31 || plantTimeStamp.hour > 23 || plantTimeStamp.minute > 59 ||
			plantTimeStamp.second > 59 || plantTimeStamp.millisecond > 999)
		{
			m_errorPlantTimeFormat++;

			if (m_timeErrLog != nullptr)
			{
				DEBUG_LOG_ERR(m_timeErrLog,
							  QString("Source %1 time format error %2").arg(moduleEquipmentID()).arg(getTimeStr(plantTimeStamp)));
			}
		}

		if (m_lastRupTimes.plant.timeStamp == m_rupTimes.plant.timeStamp)
		{
			m_errorDuplicatePlantTime++;

			if (m_timeErrLog != nullptr)
			{
				DEBUG_LOG_ERR(m_timeErrLog,
							  QString("Source %1 duplicate time %2").arg(moduleEquipmentID()).arg(getTimeStr(plantTimeStamp)));
			}
		}

		if (m_lastRupTimes.plant.timeStamp > m_rupTimes.plant.timeStamp)
		{
			m_errorNonmonotonicPlantTime++;

			if (m_timeErrLog != nullptr)
			{
				DEBUG_LOG_ERR(m_timeErrLog,
							  QString("Source %1 non monotonic time %2 (prev time %3)")
								  .arg(moduleEquipmentID())
								  .arg(getTimeStr(plantTimeStamp))
								  .arg(getTimeStr(m_lastRupTimes.plant.timeStamp)));
			}
		}

		m_lastRupTimes = m_rupTimes;
	}

	bool DataSourceOnline::takeProcessingOwnership(const QThread* processingThread)
	{
		const QThread* expected = nullptr;

		bool result = m_processingOwner.compare_exchange_strong(expected, processingThread);

		// if ownership has been taken by processingWorker - function returns TRUE
		//
		// result == FALSE is Ok, means that another thread is already take ownership

		return result;
	}

	bool DataSourceOnline::releaseProcessingOwnership(const QThread* processingThread)
	{
		bool result = m_processingOwner.compare_exchange_strong(processingThread, nullptr);

		assert(result == true); // releaseProcessingOwnership must be called by processingWorker == m_processingOwner only !!!

		return result;
	}

	QString DataSourceOnline::stateStr() const
	{
		return (m_receivesData ? "Receive data" : "No data");
	}

	QString DataSourceOnline::rupFramePlantTimeStr() const
	{
		return getTimeStr(m_lmTime);
	}

	void DataSourceOnline::clearStatistics()
	{
		m_dataReceivingSpeed = 0;
		m_receivedDataSize = 0;
		m_prevReceivedDataSize = 0;

		m_uptime = 0;             // in seconds!
		m_receivedDataID = 0;

		m_lmTime = 0;
		m_rupFrameNumerator = -1; // qint64 is Ok!

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

	bool DataSourceOnline::moveToNextWriteBuffer()
	{
		SpinLockGuard locker(&m_parsingBuffersMutex);

		m_parsingBuffers[m_writeBufferIndex]->readyToParsing = true;

		m_writeBufferIndex++;

		if (m_writeBufferIndex == m_readBufferIndex ||
			m_parsingBuffers[m_writeBufferIndex]->readyToParsing == true) // buffer is not parsed yet
		{
			m_writeBufferIndex--;                                         // return to prev writeIndexValue
			return false;
		}

		return true;
	}

	bool DataSourceOnline::moveToNextReadBuffer()
	{
		SpinLockGuard locker(&m_parsingBuffersMutex);

		if (m_parsingBuffers[m_readBufferIndex]->readyToParsing == true)
		{
			return true;
		}

		m_readBufferIndex++;

		if (m_readBufferIndex == m_writeBufferIndex ||
			m_parsingBuffers[m_readBufferIndex]->readyToParsing == false) // buffer is not raddy to parsing yet
		{
			m_readBufferIndex--;                                          // return to prev readIndexValue
			return false;
		}

		return true;
	}


	DataSourceOnline::ParsingBuffer::ParsingBuffer() {}

	DataSourceOnline::ParsingBuffer::~ParsingBuffer()
	{
		clear();
	}

	void DataSourceOnline::ParsingBuffer::clear()
	{
		framesQuantity = 0;
		DELETE_ARRAY_IF_NOT_NULL(rupFramesHeaders);
		DELETE_ARRAY_IF_NOT_NULL(rupFramesData);
		frame0ServerTime = 0;
		isSimPacket = false;
		readyToParsing = false;
	}

	void DataSourceOnline::ParsingBuffer::allocate(int frmsCount)
	{
		Q_ASSERT(frmsCount > 0 && frmsCount <= Rup::MAX_FRAME_COUNT);

		clear();

		framesQuantity = static_cast<quint16>(frmsCount);

		rupFramesHeaders = new Rup::Header[framesQuantity];
		rupFramesData = new Rup::Data[framesQuantity];

		prepareToWriting();
	}

	bool DataSourceOnline::ParsingBuffer::copyRupFrame(int frameNo, qint64 serverTime, bool simFrame, const Rup::Frame& rupFrame)
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

			for (int i = 0; i < framesQuantity; i++)
			{
				if (rupFramesHeaders[i].frameSize == 0 || rupFramesHeaders[i].numerator != numerator)
				{
					dataReadyToParsing = false;
					break;
				}
			}
		}

		return dataReadyToParsing;
	}

	void DataSourceOnline::ParsingBuffer::prepareToWriting()
	{
		for (int i = 0; i < framesQuantity; i++)
		{
			rupFramesHeaders[i].frameSize = 0;
		}

		readyToParsing = false;
	}

	const Rup::Header& DataSourceOnline::ParsingBuffer::frame0Header() const
	{
		Q_ASSERT(rupFramesHeaders != nullptr);

		return rupFramesHeaders[0];
	}

	const char* DataSourceOnline::ParsingBuffer::rupData() const
	{
		Q_ASSERT(rupFramesData != nullptr);

		return reinterpret_cast<const char*>(rupFramesData);
	}

	int DataSourceOnline::ParsingBuffer::rupDataSize() const
	{
		Q_ASSERT(framesQuantity != 0);

		return framesQuantity * sizeof(Rup::Data);
	}
} // namespace OnlineLib
