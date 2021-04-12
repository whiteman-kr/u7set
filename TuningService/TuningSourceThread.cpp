#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/Crc.h"
#include "../OnlineLib/CircularLogger.h"

#include "TuningSourceThread.h"

namespace Tuning
{
	// ----------------------------------------------------------------------------------
	//
	// SourceStatistics struct implementation
	//
	// ----------------------------------------------------------------------------------

	void SourceStatistics::get(Network::TuningSourceState* tss)
	{
		TEST_PTR_RETURN(tss);

		tss->set_sourceid(dataSourceID);

		tss->set_isreply(isReply);

		tss->set_requestcount(requestCount);
		tss->set_replycount(replyCount);

		tss->set_commandqueuesize(commandQueueSize);

		tss->set_erruntimelyreplay(errUntimelyReplay);
		tss->set_errsent(errSent);
		tss->set_errpartialsent(errPartialSent);
		tss->set_errreplysize(errReplySize);
		tss->set_errnoreply(errNoReply);
		tss->set_errrupcrc(errRupCRC);

		// errors in reply RupFrameHeader
		//
		tss->set_errrupprotocolversion(errRupProtocolVersion);
		tss->set_errrupframesize(errRupFrameSize);
		tss->set_errrupnontuningdata(errRupNonTuningData);
		tss->set_errrupmoduletype(errRupModuleType);
		tss->set_errrupframesquantity(errRupFramesQuantity);
		tss->set_errrupframenumber(errRupFrameNumber);

		// errors in reply FotipHeader
		//
		tss->set_errfotipprotocolversion(errFotipProtocolVersion);
		tss->set_errfotipuniqueid(errFotipUniqueID);
		tss->set_errfotiplmnumber(errFotipLmNumber);
		tss->set_errfotipsubsystemcode(errFotipSubsystemCode);
		tss->set_errfotipoperationcode(errFotipOperationCode);
		tss->set_errfotipframesize(errFotipFrameSize);
		tss->set_errfotipromsize(errFotipRomSize);
		tss->set_errfotipromframesize(errFotipRomFrameSize);

		// errors reported by LM in reply FotipHeader.flags
		//
		tss->set_fotipflagboundschecksuccess(fotipFlagBoundsCheckSuccess);
		tss->set_fotipflagwritesuccess(fotipFlagWriteSuccess);
		tss->set_fotipflagdatatypeerr(fotipFlagDataTypeErr);
		tss->set_fotipflagopcodeerr(fotipFlagOpCodeErr);
		tss->set_fotipflagstartaddrerr(fotipFlagStartAddrErr);
		tss->set_fotipflagromsizeerr(fotipFlagRomSizeErr);
		tss->set_fotipflagromframesizeerr(fotipFlagRomFrameSizeErr);
		tss->set_fotipflagframesizeerr(fotipFlagFrameSizeErr);
		tss->set_fotipflagprotocolversionerr(fotipFlagProtocolVersionErr);
		tss->set_fotipflagsubsystemkeyerr(fotipFlagSubsystemKeyErr);
		tss->set_fotipflaguniueiderr(fotipFlagUniueIDErr);
		tss->set_fotipflagoffseterr(fotipFlagOffsetErr);
		tss->set_fotipflagapplysuccess(fotipFlagApplySuccess);
		tss->set_fotipflagsetsor(fotipFlagSetSOR);
		tss->set_fotipflagwritingdisabled(fotipFlagWritingDisabled);

		tss->set_erranaloglowboundcheck(errAnalogLowBoundCheck);
		tss->set_erranaloghighboundcheck(errAnalogHighBoundCheck);

		tss->set_controlisactive(controlIsActive);
		tss->set_setsor(setSOR);
		tss->set_writingdisabled(writingDisabled);

		tss->set_hasunappliedparams(hasUnappliedParams);

		tss->set_errtuningframeupdate(errTuningFrameUpdate);
	}

	// ----------------------------------------------------------------------------------
	//
	// TuningSourceHandler::TuningSignal class implementation
	//
	// ----------------------------------------------------------------------------------

	void TuningSourceHandler::TuningSignal::init(const Signal* s, int index, int tuningDataFrameSizeW)
	{
		TEST_PTR_RETURN(s);

		m_appSignalID = s->appSignalID();

		m_signalType = s->signalType();
		m_analogFormat = s->analogSignalFormat();

		m_signalHash = ::calcHash(m_appSignalID);

		m_index = index;

		m_offset = s->tuningAddr().offset();
		m_bit = s->tuningAddr().bit();
		m_frameNo = s->tuningAddr().offset() / tuningDataFrameSizeW;

		updateTuningValuesType(s->signalType(), s->analogSignalFormat());

		m_lowBound = s->tuningLowBound();
		m_highBound = s->tuningHighBound();
		m_defaultValue = s->tuningDefaultValue();

		m_valid = false;
	}

	QString TuningSourceHandler::TuningSignal::tuningValueTypeStr() const
	{
		TuningValue tv(m_tuningValueType);

		return tv.typeStr();
	}

	void TuningSourceHandler::TuningSignal::updateCurrentValue(bool valid, const TuningValue& value, qint64 time)
	{
		if (valid == true)
		{
			m_successfulReadTime = time;
		}

		setCurrentValue(valid, value);
	}

	void TuningSourceHandler::TuningSignal::setCurrentValue(bool valid, const TuningValue& value)
	{
		m_valid = valid;

		assert(m_currentValue.type() == value.type());

		m_currentValue = value;
	}

	void TuningSourceHandler::TuningSignal::setReadLowBound(const TuningValue& value)
	{
		assert(m_readLowBound.type() == value.type());

		m_readLowBound = value;
	}

	void TuningSourceHandler::TuningSignal::setReadHighBound(const TuningValue& value)
	{
		assert(m_readHighBound.type() == value.type());

		m_readHighBound = value;
	}

	void TuningSourceHandler::TuningSignal::invalidate()
	{
		m_valid = false;
	}

	FotipV2::DataType TuningSourceHandler::TuningSignal::fotipV2DataType()
	{
		switch(m_tuningValueType)
		{
		case TuningValueType::Discrete:
			return FotipV2::DataType::Discrete;

		case TuningValueType::Float:
			return FotipV2::DataType::AnalogFloat;

		case TuningValueType::SignedInt32:
			return FotipV2::DataType::AnalogSignedInt;

		default:
			assert(false);
		}

		return FotipV2::DataType::Discrete;
	}

	void TuningSourceHandler::TuningSignal::updateTuningValuesType(E::SignalType signalType, E::AnalogAppSignalFormat analogFormat)
	{
		m_tuningValueType = TuningValue::getTuningValueType(signalType, analogFormat);

		m_lowBound.setType(m_tuningValueType);
		m_highBound.setType(m_tuningValueType);
		m_defaultValue.setType(m_tuningValueType);

		m_currentValue.setType(m_tuningValueType);
		m_readLowBound.setType(m_tuningValueType);
		m_readHighBound.setType(m_tuningValueType);
	}

	void TuningSourceHandler::TuningCommandQueue::push(const TuningCommand& cmd)
	{
		m_mutex.lock();

		append(cmd);

		m_mutex.unlock();
	}

	bool TuningSourceHandler::TuningCommandQueue::pop(TuningCommand* cmd)
	{
		TEST_PTR_RETURN_FALSE(cmd);

		bool result = false;

		m_mutex.lock();

		if (isEmpty() == false)
		{
			*cmd = takeFirst();
			result = true;
		}

		m_mutex.unlock();

		return result;
	}


	// ----------------------------------------------------------------------------------
	//
	// TuningSourceHandler class implementation
	//
	// ----------------------------------------------------------------------------------

	TuningSourceHandler::TuningSourceHandler(const TuningServiceSettings& settings,
										   const TuningSource& source,
										   E::SoftwareRunMode swRunMode,
										   CircularLoggerShared logger,
										   CircularLoggerShared tuningLog) :
		m_logger(logger),
		m_tuningLog(tuningLog),
		m_socket(nullptr),
		m_replyQueue(nullptr, 10)
{
		m_isSimulationMode = swRunMode == E::SoftwareRunMode::Simulation;

		m_sourceEquipmentID = source.lmEquipmentID();
		m_sourceIP = source.lmAddressPort();
		m_sourceUniqueID = source.lmUniqueID();
		m_lmNumber = static_cast<quint16>(source.lmNumber());
		m_lmModuleType = static_cast<quint16>(source.lmModuleType());
		m_subsystemCode = static_cast<quint16>(source.lmSubsystemKey());

		m_disableModulesTypeChecking = settings.disableModulesTypeChecking;

		m_tuningSimIP = settings.tuningSimIP;

		const TuningData* td = source.tuningData();

		if (td != nullptr)
		{
			m_tuningFlashSizeB = td->tuningFlashFrameCount() * td->tuningFlashFrameSizeB();
			m_tuningFlashFramePayloadB = td->tuningFlashFramePayloadB();

			m_tuningDataOffsetW = td->tuningDataOffsetW();
			m_tuningDataFrameCount = td->tuningDataFrameCount();
			m_tuningDataFramePayloadW = td->tuningDataFramePayloadW();

			m_tuningUsedFramesCount = td->usedFramesCount();
		}
		else
		{
			assert(false);
		}

		m_tuningMem.init(m_tuningDataOffsetW, m_tuningDataFramePayloadW, m_tuningDataFrameCount);

		m_stat.dataSourceID = source.ID();		// ID generated by DataSource::generateID()

		initTuningSignals(source.tuningData());
	}

	TuningSourceHandler::~TuningSourceHandler()
	{
	}

	quint32 TuningSourceHandler::sourceIP() const
	{
		return m_sourceIP.address32();
	}

	QString TuningSourceHandler::sourceEquipmentID() const
	{
		return m_sourceEquipmentID;
	}

	void TuningSourceHandler::startHandler()
	{
		m_stat.controlIsActive = true;

		DEBUG_LOG_MSG(m_logger, QString("Tuning source %1 (%2) handler is started").arg(m_sourceEquipmentID).arg(m_sourceIP.addressPortStr()));
	}

	void TuningSourceHandler::stopHandler()
	{
		m_stat.controlIsActive = false;

		DEBUG_LOG_MSG(m_logger, QString("Tuning source %1 (%2) handler is stopped").arg(m_sourceEquipmentID).arg(m_sourceIP.addressPortStr()));
	}

	void TuningSourceHandler::periodicProcessing()
	{
		// DEBUG
		m_timerCount++;

		/*
		if ((m_timerCount % 6000) == 0)			// 1 per minute
		{
			LOG_MSG(m_logger, QString("Timer is working %1 (%2) m_waitReplay = %3 m_waitReplyCounter = %4").
				arg(m_timerCount).arg(m_sourceIP.addressStr()).arg(m_waitReply).arg(m_waitReplyCounter));
		}
		*/

		// DEBUG

		if (processWaitReply() == true)
		{
			// DEBUG
			m_waitReplyFalseCount = 0;
			m_waitReplyTrueCount++;

			if((m_waitReplyTrueCount % 100) == 0)
			{
				//LOG_MSG(m_logger, QString("processWaitReply TRUE %1 (%2) m_waitReplay = %3 m_waitReplyCounter = %4").
				//		arg(m_waitReplyTrueCount).arg(m_sourceIP.addressStr()).arg(m_waitReply).arg(m_waitReplyCounter));
			}
			// DEBUG
			return;
		}

		m_waitReplyTrueCount = 0;
		m_waitReplyFalseCount++;

		if((m_waitReplyFalseCount % 100) == 0)
		{
			//LOG_MSG(m_logger, QString("processWaitReply FALSE %1 (%2) m_waitReplay = %3 m_waitReplyCounter = %4").
			//		arg(m_waitReplyFalseCount).arg(m_sourceIP.addressStr()).arg(m_waitReply).arg(m_waitReplyCounter));
		}

		if (processCommandQueue() == true)
		{
			return;
		}

		if (processIdle() == true)
		{
			m_processIdleTrueCount++;

			if((m_processIdleTrueCount % 100) == 0)
			{
				LOG_MSG(m_logger, QString("processIdle TRUE %1 (%2) m_waitReplay = %3 m_waitReplyCounter = %4").
						arg(m_processIdleTrueCount).arg(m_sourceIP.addressStr()).arg(m_waitReply).arg(m_waitReplyCounter));
			}

			return;
		}

		m_processIdleTrueCount = 0;
	}

	bool TuningSourceHandler::processReplyQueue()
	{
		assert(sizeof(Rup::Frame) == sizeof(RupFotipV2));

		// convert reply from Rup::Frame to RupFotipV2
		//
		bool res = m_replyQueue.pop(&m_reply);

		if (res == false)
		{
			return false;
		}

		if (m_waitReply == false)
		{
			m_stat.errUntimelyReplay++;
			return false;
		}

		m_waitReplyCounter = 0;
		m_retryCount = 0;

		m_stat.isReply = true;

		m_stat.replyCount++;

		if ((m_stat.replyCount % 100) == 0)
		{
			qDebug() << C_STR(QString("Receive %1 replies from %2, NoReplies = %3").
							  arg(m_stat.replyCount).arg(m_sourceEquipmentID).arg(m_stat.errNoReply));
		}

		processReply(m_reply);

		m_waitReply = false;

		return true;
	}

	void TuningSourceHandler::pushReply(const RupFotipV2& reply)
	{
		m_replyQueue.push(&reply);
	}

	void TuningSourceHandler::incErrReplySize()
	{
		m_stat.errReplySize++;
	}

	void TuningSourceHandler::getState(Network::TuningSourceState* tuningSourceState)
	{
		TEST_PTR_RETURN(tuningSourceState);

		m_stat.get(tuningSourceState);
	}

	void TuningSourceHandler::readSignalState(Network::TuningSignalState* tss) const
	{
		TEST_PTR_RETURN(tss);

		// tss.signalhash() is already filled
		//
		Hash signalHash = tss->signalhash();

		int signalIndex = m_hash2SignalIndexMap.value(signalHash, -1);

		if (signalIndex == -1)
		{
			assert(false);			// how all previous checks we pass ???

			tss->set_valid(false);
			tss->set_error(TO_INT(NetworkError::UnknownSignalHash));
			return;
		}

		const TuningSignal& ts = m_tuningSignals[signalIndex];

		tss->set_valid(ts.valid());

		bool result = true;

		result &= ts.currentValue().save(tss->mutable_value());
		result &= ts.readLowBound().save(tss->mutable_readlowbound());
		result &= ts.readHighBound().save(tss->mutable_readhighbound());

		if (result == false)
		{
			tss->set_valid(false);
			tss->set_error(TO_INT(NetworkError::InternalError));
			return;
		}

		tss->set_writeinprogress(ts.writeInProgress());

		tss->set_successfulreadtime(ts.successfulReadTime());
		tss->set_writerequesttime(ts.writeRequestTime());
		tss->set_successfulwritetime(ts.successfulWriteTime());
		tss->set_unsuccessfulwritetime(ts.unsuccessfulWriteTime());

		tss->set_writeclient(ts.writeClient());
		tss->set_writeerrorcode(TO_INT(ts.writeErrorCode()));

		tss->set_setsor(m_stat.setSOR);
		tss->set_writingdisabled(m_stat.writingDisabled);

		tss->set_error(TO_INT(NetworkError::Success));
	}

	NetworkError TuningSourceHandler::writeSignalState(	const QString& clientEquipmentID,
														const QString& user,
														Hash signalHash,
														const TuningValue& newValue)
	{
		int signalIndex = m_hash2SignalIndexMap.value(signalHash, -1);

		if (signalIndex == -1)
		{
			return NetworkError::UnknownSignalHash;
		}

		if (signalIndex < 0 || signalIndex >= m_tuningSignals.count())
		{
			assert(false);

			DEBUG_LOG_ERR(m_logger, "Signal index out of range (TuningSourceHandler::writeSignalState)");

			return NetworkError::InternalError;
		}

		TuningSignal& ts = m_tuningSignals[signalIndex];

		if (ts.tuningValueType() != newValue.type())
		{
			DEBUG_LOG_ERR(m_logger, QString("Tuning value type (%1) is not correspond to tuning signal %2 type (%3)").
											arg(newValue.typeStr()).
											arg(ts.appSignalID()).
											arg(ts.tuningValueTypeStr()));

			return NetworkError::WrongTuningValueType;
		}

		if (newValue < ts.lowBound() || newValue > ts.highBound())
		{
			DEBUG_LOG_ERR(m_logger, QString("New tuning value (%1) of tuning signal %2 is out of range (%3..%4)").
											arg(newValue.doubleValue()).
											arg(ts.appSignalID()).
											arg(ts.lowBound().toString()).
											arg(ts.highBound().toString()));

			return NetworkError::TuningValueOutOfRange;
		}

		TuningCommand cmd;

		cmd.clientEquipmentID = clientEquipmentID;
		cmd.user = user;

		cmd.opCode = FotipV2::OpCode::Write;
		cmd.autoCommand = false;

		cmd.write.signalIndex = signalIndex;
		cmd.write.newTuningValue = newValue;

		m_tuningCommandQueue.push(cmd);

		LOG_MSG(m_logger, QString("Queue write command: source %1 (%2), signal %3, value %4").
					  arg(sourceEquipmentID()).
					  arg(m_sourceIP.addressStr()).
					  arg(m_tuningSignals[signalIndex].appSignalID()).
					  arg(newValue.toString()));

		return NetworkError::Success;
	}

	NetworkError TuningSourceHandler::applySignalStates(	const QString& clientEquipmentID,
														const QString& user)
	{
		TuningCommand cmd;

		cmd.clientEquipmentID = clientEquipmentID;
		cmd.user = user;

		cmd.opCode = FotipV2::OpCode::Apply;
		cmd.autoCommand = false;

		m_tuningCommandQueue.push(cmd);

		DEBUG_LOG_MSG(m_logger, QString("Queue apply command: source %1 (%2)").
					  arg(sourceEquipmentID()).
					  arg(m_sourceIP.addressStr()));

		return NetworkError::Success;
	}

	void TuningSourceHandler::initTuningSignals(const TuningData* td)
	{
		m_tuningSignals.clear();
		m_hash2SignalIndexMap.clear();
		m_frameSignals.clear();

		if (td == nullptr)
		{
			assert(false);
			return;
		}

		QVector<Signal*> tuningSignals;

		td->getSignals(tuningSignals);

		int signalCount = tuningSignals.count();

		m_tuningSignals.resize(signalCount);
		m_hash2SignalIndexMap.reserve(static_cast<int>(signalCount * 1.2));

		for(int i = 0; i < signalCount; i++)
		{
			Signal* signal = tuningSignals[i];

			if (signal == nullptr)
			{
				assert(false);
				continue;
			}

			Hash hash = calcHash(signal->appSignalID());

			if (m_hash2SignalIndexMap.contains(hash) == true)
			{
				assert(false);
				continue;
			}

			m_hash2SignalIndexMap.insert(hash, i);

			TuningSignal& ts = m_tuningSignals[i];

			ts.init(signal, i, m_tuningDataFramePayloadW);

			int arrayIndex = ts.frameNo() / 3;

			assert(arrayIndex <= m_tuningDataFrameCount / 3);

			while (arrayIndex >= m_frameSignals.count())			// appends new arrays if need
			{
				m_frameSignals.append(QVector<int>());
			}

			m_frameSignals[arrayIndex].append(i);
		}
	}

	bool TuningSourceHandler::processWaitReply()
	{
		if (m_waitReply == true)
		{
			m_waitReplyCounter++;

			if (m_waitReplyCounter < MAX_WAIT_REPLY_COUNTER)
			{
				return true;
			}

			m_waitReplyCounter = 0;

			// fix replay timeout
			//
			m_stat.errNoReply++;

			m_waitReply = false;

			qDebug() << "NoReply " << C_STR(m_sourceIP.addressStr());

			m_retryCount++;

			if (m_retryCount >= MAX_RETRY_COUNT)
			{
				// fix - source is not reply
				//
				onNoReply();

				m_stat.isReply = false;

				invalidateAllSignals();

				qDebug() << "Invalidate signals on NoReply " << C_STR(m_sourceIP.addressStr());
			}
			else
			{
				// retry last request
				//
				sendFotipRequest(m_request, m_requestAppSignalID);
			}
		}

		return false;			// switch to next processing
	}

	bool TuningSourceHandler::processCommandQueue()
	{
		if (m_waitReply == true)
		{
			return true;		// while wating reply has not another processing
		}

		// get command from queue and send FOTIP request
		//

		if (m_tuningCommandQueue.pop(&m_lastProcessedCommand) == false)
		{
			return false;		// queue is empty, go to next processing
		}

		bool result = prepareFotipRequest(m_lastProcessedCommand, m_request.rupFotipV2);

		if (result == false)
		{
			return false;
		}

		m_requestAppSignalID.clear();

		logTuningRequest(m_lastProcessedCommand, &m_requestAppSignalID);

		m_retryCount = 0;

		sendFotipRequest(m_request, m_requestAppSignalID);

		return true;
	}

	bool TuningSourceHandler::processIdle()
	{
		if (m_waitReply == true)
		{
			return true;		// while wating reply has not another processing
		}

		TuningCommand tuningCmd;

		tuningCmd.opCode = FotipV2::OpCode::Read;
		tuningCmd.read.frame = m_nextFrameToAutoRead;
		tuningCmd.autoCommand = true;

		m_tuningCommandQueue.push(tuningCmd);

		m_nextFrameToAutoRead++;

		if (m_nextFrameToAutoRead >= m_tuningUsedFramesCount)
		{
			m_nextFrameToAutoRead = 0;
		}

		return false;
	}

	void TuningSourceHandler::onNoReply()
	{
		finalizeWriting(NetworkError::TuningNoReply);
	}

	bool TuningSourceHandler::prepareFotipRequest(const TuningCommand& tuningCmd, RupFotipV2 &request)
	{
		bool result = true;

		result &= initRupHeader(request.rupHeader);

		result &= initFotipFrame(request.fotipFrame, tuningCmd);

		return result;
	}

	void TuningSourceHandler::sendFotipRequest(SimRupFotipV2& request, const QString& appSignalID)
	{
		assert(sizeof(Rup::Frame) == Socket::ENTIRE_UDP_SIZE);
		assert(sizeof(RupFotipV2) == Socket::ENTIRE_UDP_SIZE);
		assert(sizeof(FotipV2::Frame) == Rup::FRAME_DATA_SIZE);
		assert(sizeof(FotipV2::Header) == 128);

		RupFotipV2& rupFotipV2 = request.rupFotipV2;

		// convert headers to BigEndian
		//
		rupFotipV2.rupHeader.reverseBytes();
		rupFotipV2.fotipFrame.header.reverseBytes();

		//

		rupFotipV2.calcCRC64();

		qint64 sent = 0;

		if (m_isSimulationMode == false)
		{
			// packet sending to real LM
			//
			sent = m_socket.writeDatagram(reinterpret_cast<char*>(&rupFotipV2),
										  sizeof(rupFotipV2),
										  m_sourceIP.address(),
										  m_sourceIP.port());
		}
		else
		{
			// packet sending to Simulator
			//
			request.simVersion = reverseUint16(1);
			request.tuningSourceIP = reverseUint32(m_sourceIP.address32());

			sent = m_socket.writeDatagram(reinterpret_cast<char*>(&request),
										  sizeof(request),
										  m_tuningSimIP.address(),
										  m_tuningSimIP.port());
		}

		m_stat.requestCount++;

		// revert headers to LittleEndian
		//
		rupFotipV2.rupHeader.reverseBytes();
		rupFotipV2.fotipFrame.header.reverseBytes();

		//

		quint32 rawDiscreteValue = rupFotipV2.fotipFrame.write.discreteValue;
		quint32 rawBitmask = rupFotipV2.fotipFrame.write.bitMask;
		quint16 requestID = rupFotipV2.rupHeader.numerator;

		//

		m_waitReplyCounter = 0;

		m_waitReply = true;

		if (sent == -1)
		{
			m_stat.errSent++;
			return;
		}

		if (sent < static_cast<qint64>(sizeof(m_request)))
		{
			m_stat.errPartialSent++;
		}

		// logging
		//
		switch(static_cast<FotipV2::OpCode>(rupFotipV2.fotipFrame.header.operationCode))
		{
		case FotipV2::OpCode::Write:
			{
				QString valueStr = rupFotipV2.fotipFrame.valueStr(true);

				if (rupFotipV2.fotipFrame.isDiscreteData() == true)
				{
					DEBUG_LOG_MSG(m_logger, QString("RupFotipV2 WRITE request %1 is sent to %2 (%3), signal %4 value %5."
													"StartAddrW %6, OffsetInFrameW %7, RawValue32 %8 BE, Bitmask32 %9 BE").
								  arg(requestID, 4, 16, QLatin1Char('0')).
								  arg(sourceEquipmentID()).
								  arg(m_sourceIP.addressStr()).
								  arg(appSignalID).
								  arg(valueStr).
								  arg(rupFotipV2.fotipFrame.header.startAddressW).
								  arg(rupFotipV2.fotipFrame.header.offsetInFrameW).
								  arg(rawDiscreteValue, 8, 16, QLatin1Char('0')).
								  arg(rawBitmask, 8, 16, QLatin1Char('0')));
				}
				else
				{
					DEBUG_LOG_MSG(m_logger, QString("RupFotipV2 WRITE request %1 is sent to %2 (%3), signal %4 value %5").
								  arg(requestID, 4, 16, QLatin1Char('0')).
								  arg(sourceEquipmentID()).
								  arg(m_sourceIP.addressStr()).
								  arg(appSignalID).
								  arg(valueStr));
				}
			}
			break;

		case FotipV2::OpCode::Apply:
			DEBUG_LOG_MSG(m_logger, QString("RupFotipV2 APPLY request %1 is sent to %2 (%3)").
						  arg(requestID, 4, 16, QLatin1Char('0')).
						  arg(sourceEquipmentID()).
						  arg(m_sourceIP.addressStr()));
			break;

		case FotipV2::OpCode::Read:
			// no log read request
			break;

		default:
			assert(false);
		}
	}

	bool TuningSourceHandler::initRupHeader(Rup::Header& rupHeader)
	{
		rupHeader.frameSize = Socket::ENTIRE_UDP_SIZE;
		rupHeader.protocolVersion = Rup::VERSION;

		rupHeader.flags.all = 0;
		rupHeader.flags.tuningData = 1;

		rupHeader.dataId = 0;
		rupHeader.moduleType = m_lmModuleType;
		rupHeader.numerator = m_rupNumerator;
		rupHeader.framesQuantity = 1;
		rupHeader.frameNumber = 0;

		rupHeader.timeStamp.setDateTime(QDateTime::currentDateTime());

		m_rupNumerator++;

		return true;
	}

	bool TuningSourceHandler::initFotipFrame(FotipV2::Frame& fotipFrame, const TuningCommand& tuningCmd)
	{
		FotipV2::Header& fotipHeader = fotipFrame.header;

		// common initialization
		//
		fotipHeader.protocolVersion = FotipV2::VERSION;
		fotipHeader.uniqueId = m_sourceUniqueID;

		fotipHeader.subsystemKey.wordVaue = 0;
		fotipHeader.subsystemKey.lmNumber = m_lmNumber;
		fotipHeader.subsystemKey.subsystemCode = m_subsystemCode;
		fotipHeader.subsystemKey.crc = Crc::crc4(fotipHeader.subsystemKey.wordVaue);

		fotipHeader.flags.all = 0;

		fotipHeader.fotipFrameSizeB = sizeof(FotipV2::Frame);

		fotipHeader.romSizeB = static_cast<quint32>(m_tuningFlashSizeB);
		fotipHeader.romFrameSizeB = static_cast<quint16>(m_tuningFlashFramePayloadB);

		fotipHeader.offsetInFrameW = 0;

		memset(fotipHeader.reserv, 0, sizeof(fotipHeader.reserv));

		memset(fotipFrame.data, 0, sizeof(fotipFrame.data));

		memset(&fotipFrame.analogCmpErrors, 0, sizeof(fotipFrame.analogCmpErrors));

		memset(fotipFrame.reserv, 0, sizeof(fotipFrame.reserv));

		//

		fotipHeader.operationCode = static_cast<quint16>(tuningCmd.opCode);

		// operation-specific initialization
		//
		switch(tuningCmd.opCode)
		{
		case FotipV2::OpCode::Read:
			fotipHeader.startAddressW = m_tuningDataOffsetW + tuningCmd.read.frame * m_tuningDataFramePayloadW;
			fotipHeader.dataType = TO_INT(FotipV2::DataType::Discrete);		// any data type is allowed
			break;

		case FotipV2::OpCode::Write:
			{
				int signalIndex = tuningCmd.write.signalIndex;
				int signalCount = m_tuningSignals.count();

				if (signalIndex < 0 || signalIndex >= signalCount)
				{
					assert(false);
					return false;
				}

				TuningSignal& ts = m_tuningSignals[signalIndex];

				int offsetW = ts.offset();

				int frameNo =  offsetW / m_tuningDataFramePayloadW;

				if ((frameNo % 3) != 0)
				{
					assert(false);
					return false;
				}

				fotipHeader.dataType = static_cast<quint16>(ts.fotipV2DataType());

				fotipHeader.startAddressW = m_tuningDataOffsetW + frameNo * m_tuningDataFramePayloadW;
				fotipHeader.offsetInFrameW = offsetW - frameNo * m_tuningDataFramePayloadW;

				fotipFrame.write.bitMask = 0;
				fotipFrame.write.discreteValue = 0;		// zero fotipFrame.write.floatValue also

				switch(ts.tuningValueType())
				{
				case TuningValueType::Float:
					fotipFrame.write.analogFloatValue = reverseFloat(tuningCmd.write.newTuningValue.floatValue());
					break;

				case TuningValueType::SignedInt32:
					fotipFrame.write.analogSignedIntValue = reverseInt32(tuningCmd.write.newTuningValue.int32Value());
					break;

				case TuningValueType::Discrete:
					{
						int bit = ts.bit();

						assert(bit >= 0 && bit < 32 );

						quint32 bitmask = 1 << bit;

						fotipFrame.write.bitMask = reverseUint32(bitmask);

						quint32 discreteValue = tuningCmd.write.newTuningValue.discreteValue() << bit;

						fotipFrame.write.discreteValue = reverseUint32(discreteValue);
					}
					break;

				default:
					assert(false);
				}

				ts.setWriteRequestTime(QDateTime::currentMSecsSinceEpoch());
				ts.setWriteClient(tuningCmd.clientEquipmentID);
				ts.resetWriteErrorCode();
				ts.setWriteInProgress(true);
			}
			break;

		case FotipV2::OpCode::Apply:
			break;

		default:
			assert(false);
			return false;
		}

		return true;
	}

	void TuningSourceHandler::processReply(RupFotipV2& reply)
	{
		bool result = true;

		result = reply.checkCRC64();

		if (result == false)
		{
			finalizeWriting(NetworkError::TuningNoReply);
			m_stat.errRupCRC++;
			return;
		}

		reply.rupHeader.reverseBytes();
		reply.fotipFrame.header.reverseBytes();

		result = checkRupHeader(reply.rupHeader);

		if (result == false)
		{
			finalizeWriting(NetworkError::TuningNoReply);
			return;
		}

		FotipV2::Header& fotipHeader = reply.fotipFrame.header;

		result = checkFotipHeader(fotipHeader);

		if (result == false)
		{
			finalizeWriting(NetworkError::TuningNoReply);
			return;
		}

		switch(static_cast<FotipV2::OpCode>(fotipHeader.operationCode))
		{
		case FotipV2::OpCode::Read:
			processReadReply(reply);
			break;

		case FotipV2::OpCode::Write:
			processWriteReply(reply);
			break;

		case FotipV2::OpCode::Apply:
			processApplyReply(reply);
			break;

		default:
			assert(false);
		}
	}

	void TuningSourceHandler::processReadReply(RupFotipV2& reply)
	{
		updateFrameSignalsState(reply);
	}

	void TuningSourceHandler::processWriteReply(RupFotipV2& reply)
	{
		updateFrameSignalsState(reply);

		reply.fotipFrame.analogCmpErrors.all = reverseUint16(reply.fotipFrame.analogCmpErrors.all);

		QString msg;

		bool hasErrors = false;

		NetworkError errCode = NetworkError::Success;

		switch(static_cast<FotipV2::DataType>(reply.fotipFrame.header.dataType))
		{
		case FotipV2::DataType::AnalogFloat:
		case FotipV2::DataType::AnalogSignedInt:
			{
				QString boundCheckStr;

				if (reply.fotipFrame.analogCmpErrors.highBoundCheckError == 1)
				{
					m_stat.errAnalogHighBoundCheck++;

					boundCheckStr = QString("HighBoundCheckError == 1 ");
					errCode = NetworkError::TuningValueOutOfRange;
					hasErrors = true;
				}

				if (reply.fotipFrame.analogCmpErrors.lowBoundCheckError == 1)
				{
					m_stat.errAnalogLowBoundCheck++;

					boundCheckStr = QString("LowBoundCheckError == 1 ");
					errCode = NetworkError::TuningValueOutOfRange;
					hasErrors = true;
				}

				if (reply.fotipFrame.analogCmpErrors.highBoundCheckError == 0 &&
					reply.fotipFrame.analogCmpErrors.lowBoundCheckError == 0)
				{
					boundCheckStr = ("No bound check errors ");
				}

				msg = QString("Reply is received from %1 (%2) on RupFotipV2 WRITE request %3: %4").
								arg(sourceEquipmentID()).
								arg(m_sourceIP.addressStr()).
								arg(reply.rupHeader.numerator, 4, 16, QLatin1Char('0')).
								arg(boundCheckStr);
			}
			break;

		case FotipV2::DataType::Discrete:
			{
				quint32 data32 = *reinterpret_cast<quint32*>(reply.fotipFrame.data + m_request.rupFotipV2.fotipFrame.header.offsetInFrameW * 2);

				msg = QString("Reply is received from %1 (%2) on RupFotipV2 WRITE request %3. Data32[%4W] = %5").
								arg(sourceEquipmentID()).
								arg(m_sourceIP.addressStr()).
								arg(reply.rupHeader.numerator, 4, 16, QLatin1Char('0')).
								arg(m_request.rupFotipV2.fotipFrame.header.offsetInFrameW).
								arg(data32, 8, 16, QLatin1Char('0'));
			}
			break;

		default:
			assert(false);
		}

		TuningValue& newTuningValue = m_lastProcessedCommand.write.newTuningValue;

		TuningValue currentValue = m_tuningSignals[m_lastProcessedCommand.write.signalIndex].currentValue();

		if (newTuningValue != currentValue)
		{
			errCode = NetworkError::TuningValueCorrupted;

			msg +=  QString("Tuning value corrupted");

			hasErrors = true;
		}

		finalizeWriting(errCode);

		DEBUG_LOG_ERR(m_logger, msg);

		if (hasErrors == true)
		{
//			DEBUG_LOG_ERR(m_logger, msg);
		}
		else
		{
			m_stat.hasUnappliedParams = true;
		}

		logTuningReply(m_lastProcessedCommand, reply);
	}

	void TuningSourceHandler::processApplyReply(RupFotipV2& reply)
	{
		QString result;

		if (reply.fotipFrame.header.flags.succesfulApply == 1)
		{
			result = "Success";

			m_stat.hasUnappliedParams = false;
		}
		else
		{
			result = "Fail";
		}

		DEBUG_LOG_MSG(m_logger, QString("Reply is received from %1 (%2) on RupFotipV2 APPLY request %3: %4").
					  arg(sourceEquipmentID()).
					  arg(m_sourceIP.addressStr()).
					  arg(reply.rupHeader.numerator, 4, 16, QLatin1Char('0')).
					  arg(result));

		logTuningReply(m_lastProcessedCommand, reply);
	}

	void TuningSourceHandler::updateFrameSignalsState(RupFotipV2& reply)
	{
		bool updateResult = m_tuningMem.updateFrame(reply.fotipFrame.header.startAddressW,
													reply.fotipFrame.header.romFrameSizeB,
													reply.fotipFrame.data);
		if (updateResult == false)
		{
			m_stat.errTuningFrameUpdate++;
			return;
		}

		// parse signals values and bounds
		//
		int frameNo = (reply.fotipFrame.header.startAddressW - m_tuningDataOffsetW) / m_tuningDataFramePayloadW;

		int arrayIndex = frameNo / 3;

		if (arrayIndex < 0 || arrayIndex >= m_frameSignals.count())
		{
			assert(false);
			return;
		}

		const QVector<int>& frameSignals = m_frameSignals[arrayIndex];

		quint8* dataPtr = reply.fotipFrame.data;

		qint64 updateTime = QDateTime::currentMSecsSinceEpoch();

		int tuningSignalsCount = m_tuningSignals.count();

		for(int signalIndex : frameSignals)
		{
			if (signalIndex < 0 || signalIndex >= tuningSignalsCount)
			{
				assert(false);
				continue;
			}

			TuningSignal& ts = m_tuningSignals[signalIndex];

			TuningValueType tyningValueType = ts.tuningValueType();

			TuningValue tuningValue(tyningValueType);

			int offsetInFrameB = (ts.offset() - ts.frameNo() * m_tuningDataFramePayloadW) * sizeof(quint16);

			assert(offsetInFrameB < reply.fotipFrame.header.romFrameSizeB);

			switch(tyningValueType)
			{
			case TuningValueType::Float:
				tuningValue.setFloatValue(reverseFloat(*reinterpret_cast<float*>(dataPtr + offsetInFrameB)));
				break;

			case TuningValueType::SignedInt32:
				tuningValue.setInt32Value(reverseInt32(*reinterpret_cast<qint32*>(dataPtr + offsetInFrameB)));
				break;

			case TuningValueType::Double:
				assert(false);					// is not implemented
				break;

			case TuningValueType::Discrete:
				{
					quint32 word =	reverseUint32(*reinterpret_cast<quint32*>(dataPtr + offsetInFrameB));
					tuningValue.setDiscreteValue((word & (1 << ts.bit())) == 0 ? 0 : 1);
				}
				break;

			default:
				assert(false);					// unknown type
			}

			// (frameNo % 3) == 0 - tuning signal value
			// (frameNo % 3) == 1 - tuning signal read low bound
			// (frameNo % 3) == 2 - tuning signal read high bound

			switch(frameNo % 3)
			{
			case 0:
				ts.updateCurrentValue(true, tuningValue, updateTime);
				break;

			case 1:
				ts.setReadLowBound(tuningValue);
				break;

			case 2:
				ts.setReadHighBound(tuningValue);
				break;

			default:
				assert(false);
			}
		}
	}

	void TuningSourceHandler::finalizeWriting(NetworkError errCode)
	{
		if (m_lastProcessedCommand.opCode != FotipV2::OpCode::Write)
		{
			return;
		}

		int index = m_lastProcessedCommand.write.signalIndex;

		if (index < 0 || index >= m_tuningSignals.count())
		{
			assert(false);
			return;
		}

		TuningSignal& ts = m_tuningSignals[index];

		qint64 time = QDateTime::currentMSecsSinceEpoch();

		if (errCode == NetworkError::Success)
		{
			ts.setSuccessfulWriteTime(time);
			ts.setUnsuccessfulWriteTime(0);
		}
		else
		{
			ts.setSuccessfulWriteTime(0);
			ts.setUnsuccessfulWriteTime(time);
		}

		ts.setWriteErrorCode(errCode);
		ts.setWriteInProgress(false);
	}

	bool TuningSourceHandler::checkRupHeader(const Rup::Header& rupHeader)
	{
		bool result = true;

		if (rupHeader.protocolVersion != Rup::VERSION)
		{
			m_stat.errRupProtocolVersion++;
			result &= false;
		}

		if (rupHeader.frameSize != Socket::ENTIRE_UDP_SIZE)
		{
			m_stat.errRupFrameSize++;
			result &= false;
		}

		if (rupHeader.flags.tuningData != 1 ||
			rupHeader.flags.appData != 0 ||
			rupHeader.flags.diagData != 0 ||
			rupHeader.flags.test != 0)
		{
			m_stat.errRupNonTuningData++;
			result &= false;
		}

		if (m_disableModulesTypeChecking == false && rupHeader.moduleType != m_lmModuleType)
		{
			m_stat.errRupModuleType++;
			result &= false;

			qDebug() << "Invalid moduleType of" << m_sourceEquipmentID << "( waiting" << m_lmModuleType << ", receiving" << rupHeader.moduleType << ")";
		}

		if (rupHeader.framesQuantity != 1)
		{
			m_stat.errRupFramesQuantity++;
			result &= false;
		}

		if (rupHeader.frameNumber != 0)
		{
			m_stat.errRupFrameNumber++;
			result &= false;
		}

		//	quint32 dataId;	??

		return result;
	}

	bool TuningSourceHandler::checkFotipHeader(const FotipV2::Header& fotipHeader)
	{
		bool result = true;

		if (fotipHeader.protocolVersion != FotipV2::VERSION)
		{
			m_stat.errFotipProtocolVersion++;
			result = false;
		}

		if (fotipHeader.uniqueId != m_sourceUniqueID)
		{
			m_stat.errFotipUniqueID++;

			if ((m_stat.errFotipUniqueID % 500) == 0)
			{
				DEBUG_LOG_ERR(m_logger, QString("Wrong tuning source UniqueID: %1.").arg(m_sourceIP.addressStr()));
			}
			result = false;
		}
		else
		{
			m_stat.errFotipUniqueID = 0;		// added by Vintenko 26.12.2017
		}

		if (fotipHeader.subsystemKey.lmNumber != m_lmNumber)
		{
			m_stat.errFotipLmNumber++;
			result = false;
		}

		if (fotipHeader.subsystemKey.subsystemCode != m_subsystemCode)
		{
			m_stat.errFotipSubsystemCode++;
			result = false;
		}

		if (fotipHeader.operationCode != m_request.rupFotipV2.fotipFrame.header.operationCode)
		{
			m_stat.errFotipOperationCode++;
			result = false;
		}

		if (fotipHeader.fotipFrameSizeB != sizeof(FotipV2::Frame))
		{
			m_stat.errFotipFrameSize++;
			result = false;
		}

		if (fotipHeader.romSizeB !=  static_cast<quint32>(m_tuningFlashSizeB))
		{
			m_stat.errFotipRomSize++;
			result = false;
		}

		if (fotipHeader.romFrameSizeB != m_tuningFlashFramePayloadB)
		{
			m_stat.errFotipRomFrameSize++;
			result = false;
		}

		const FotipV2::HeaderFlags& flags = fotipHeader.flags;

		// check FOTIP error flags
		//
		if (flags.dataTypeError == 1)
		{
			m_stat.fotipFlagDataTypeErr++;
			result = false;
		}

		if (flags.operationCodeError == 1)
		{
			m_stat.fotipFlagOpCodeErr++;
			result = false;
		}

		if (flags.startAddressError == 1)
		{
			m_stat.fotipFlagStartAddrErr++;
			result = false;
		}

		if (flags.romSizeError == 1)
		{
			m_stat.fotipFlagRomSizeErr++;
			result = false;
		}

		if (flags.romFrameSizeError == 1)
		{
			m_stat.fotipFlagRomFrameSizeErr++;
			result = false;
		}

		if (flags.frameSizeError == 1)
		{
			m_stat.fotipFlagFrameSizeErr++;
			result = false;
		}

		if (flags.versionError == 1)
		{
			m_stat.fotipFlagProtocolVersionErr++;
			result = false;
		}

		if (flags.subsystemKeyError == 1)
		{
			m_stat.fotipFlagSubsystemKeyErr++;
			result = false;
		}

		if (flags.idError == 1)
		{
			m_stat.fotipFlagUniueIDErr++;
			result = false;
		}

		if (flags.offsetError == 1)
		{
			m_stat.fotipFlagOffsetErr++;
			result = false;
		}

		// check FOTIP success flags
		//
		if (flags.successfulCheck == 1)
		{
			m_stat.fotipFlagBoundsCheckSuccess++;
		}

		if (flags.successfulWrite == 1)
		{
			m_stat.fotipFlagWriteSuccess++;
		}

		if (flags.succesfulApply == 1)
		{
			m_stat.fotipFlagApplySuccess++;
		}

		if (flags.setSOR == 1)
		{
			m_stat.fotipFlagSetSOR++;					// for platform LMs
			m_stat.setSOR = true;

			m_stat.fotipFlagWritingDisabled++;			// for non-platform LMs
			m_stat.writingDisabled = true;
		}
		else
		{
			m_stat.fotipFlagSetSOR = 0;					// for platform LMs
			m_stat.setSOR = false;

			m_stat.fotipFlagWritingDisabled = 0;		// for non-platform LMs
			m_stat.writingDisabled = false;
		}

		return result;
	}

	void TuningSourceHandler::invalidateAllSignals()
	{
		for(TuningSignal& s : m_tuningSignals)
		{
			s.invalidate();
		}
	}

	void TuningSourceHandler::logTuningRequest(const TuningCommand& cmd, QString* appSignalID)
	{
		TEST_PTR_RETURN(appSignalID);

		QString str = QString("LM=%1 Client=%2 User=%3").arg(m_sourceEquipmentID).arg(cmd.clientEquipmentID).arg(cmd.user);

		QString logStr;

		switch(cmd.opCode)
		{
		case FotipV2::OpCode::Read:
			return;

		case FotipV2::OpCode::Write:
			{
				const TuningSignal& ts = m_tuningSignals[cmd.write.signalIndex];

				ts.currentValue();

				logStr = QString("WRITE request %1 Signal=%2 Type=%3 CurValue=%4 NewValue=%5 SOR=%6").
							arg(str).arg(ts.appSignalID()).arg(cmd.write.newTuningValue.typeStr()).
							arg(ts.currentValue().toString()).arg(cmd.write.newTuningValue.toString()).
							arg(m_stat.setSOR == true ? 1 : 0);

				*appSignalID = ts.appSignalID();
			}
			break;

		case FotipV2::OpCode::Apply:
			logStr = QString("APPLY request %1 SOR=%2").arg(str).arg(m_stat.setSOR == true ? 1 : 0);
			break;

		default:
			assert(false);
			return;
		}

		LOG_MSG(m_tuningLog, logStr);
	}

	void TuningSourceHandler::logTuningReply(const TuningCommand& cmd, const RupFotipV2& reply)
	{
		QString logStr;

		switch(cmd.opCode)
		{
		case FotipV2::OpCode::Read:
			return;

		case FotipV2::OpCode::Write:
			{
				const TuningSignal& ts = m_tuningSignals[cmd.write.signalIndex];

				ts.currentValue();

				QString checkResultStr;

				if (ts.signalType() == E::SignalType::Analog)
				{
					checkResultStr = QString("LowBoundCheck=%1 HighBoundCheck=%2 ").
							arg(reply.fotipFrame.analogCmpErrors.lowBoundCheckError == 0 ? "Success" : "Fail").
							arg(reply.fotipFrame.analogCmpErrors.highBoundCheckError == 0 ? "Success" : "Fail");
				}

				logStr = QString("WRITE reply&nbsp;&nbsp;&nbsp;LM=%1 Signal=%2 CurValue=%3 %4SOR=%5").
							arg(m_sourceEquipmentID).
							arg(ts.appSignalID()).
							arg(ts.currentValue().toString()).
							arg(checkResultStr).
							arg(reply.fotipFrame.header.flags.setSOR);
			}
			break;

		case FotipV2::OpCode::Apply:
			logStr = QString("APPLY reply&nbsp;&nbsp;&nbsp;LM=%1 Result=%2 SOR=%3").
						arg(m_sourceEquipmentID).
						arg(reply.fotipFrame.header.flags.succesfulApply == 1 ? "Success" : "Fail").
						arg(m_stat.setSOR == true ? 1 : 0);
			break;

		default:
			assert(false);
			return;
		}

		LOG_MSG(m_tuningLog, logStr);
	}


	// ----------------------------------------------------------------------------------
	//
	// TuningSourceThread class implementation (QThread::run override)
	//
	// ----------------------------------------------------------------------------------

	TuningSourceThread::TuningSourceThread(const TuningServiceSettings& settings,
											const TuningSource& source,
											E::SoftwareRunMode swRunMode,
											CircularLoggerShared logger,
											CircularLoggerShared tuningLog) :
		m_settings(settings),
		m_source(source),
		m_swRunMode(swRunMode),
		m_logger(logger),
		m_tuningLog(tuningLog)
	{
	}

	void TuningSourceThread::pushReply(const RupFotipV2& reply)
	{
		AUTO_LOCK(m_handlerMutex);

		if (m_handler != nullptr)
		{
			m_handler->pushReply(reply);
		}
	}

	void TuningSourceThread::incErrReplySize()
	{
		AUTO_LOCK(m_handlerMutex);

		if (m_handler != nullptr)
		{
			m_handler->incErrReplySize();
		}
	}

	void TuningSourceThread::getState(Network::TuningSourceState* tuningSourceState)
	{
		AUTO_LOCK(m_handlerMutex);

		if (m_handler != nullptr)
		{
			m_handler->getState(tuningSourceState);
		}
	}

	void TuningSourceThread::readSignalState(Network::TuningSignalState* tss) const
	{
		AUTO_LOCK(m_handlerMutex);

		if (m_handler != nullptr)
		{
			m_handler->readSignalState(tss);
		}
	}

	NetworkError TuningSourceThread::writeSignalState(	const QString& clientEquipmentID,
														const QString& user,
														Hash signalHash,
														const TuningValue& newValue)
	{
		AUTO_LOCK(m_handlerMutex);

		if (m_handler != nullptr)
		{
			return m_handler->writeSignalState(clientEquipmentID, user, signalHash, newValue);
		}

		return NetworkError::LmControlIsNotActive;
	}

	NetworkError TuningSourceThread::applySignalStates(	const QString& clientEquipmentID,
														const QString& user)
	{
		AUTO_LOCK(m_handlerMutex);

		if (m_handler != nullptr)
		{
			return m_handler->applySignalStates(clientEquipmentID, user);
		}

		return NetworkError::LmControlIsNotActive;
	}

	QString TuningSourceThread::sourceEquipmentID() const
	{
		AUTO_LOCK(m_handlerMutex);

		if (m_handler != nullptr)
		{
			return m_handler->sourceEquipmentID();
		}

		return QString();
	}

	void TuningSourceThread::waitWhileHandlerInitialized() const
	{
		while(m_handlerIsInitialized.load() != true)
		{
			QThread::yieldCurrentThread();
		}
	}

	void TuningSourceThread::run()
	{
		initHandler();

		int msCount = 0;

		do
		{
			msCount++;

			bool replyProcessed = m_handler->processReplyQueue();

			if (msCount >= 10 || replyProcessed == true)
			{
				m_handler->periodicProcessing();	// each 10 ms or after reply processed

				msCount = 0;
			}

			msleep(1);
		}
		while(isQuitRequested() == false);

		shutdownHandler();
	}

	void TuningSourceThread::initHandler()
	{
		AUTO_LOCK(m_handlerMutex);

		m_handler = new TuningSourceHandler(m_settings, m_source, m_swRunMode, m_logger, m_tuningLog);

		m_handler->startHandler();

		m_handlerIsInitialized.store(true);
	}

	void TuningSourceThread::shutdownHandler()
	{
		AUTO_LOCK(m_handlerMutex);

		if (m_handler != nullptr)
		{
			m_handler->stopHandler();

			delete m_handler;

			m_handler = nullptr;
		}
		else
		{
			assert(false);
		}
	}

	// -------------------------------------------------------------------------
	//
	//	TuningSocketListener class implementaton
	//
	// -------------------------------------------------------------------------

	TuningSocketListener::TuningSocketListener(const HostAddressPort& listenIP,
											   TuningSourceThreadMap& sourceWorkerMap,
											   bool simulationMode,
											   std::shared_ptr<CircularLogger> logger) :
		m_listenIP(listenIP),
		m_sourceThreadMap(sourceWorkerMap),
		m_simMode(simulationMode),
		m_logger(logger),
		m_timer(this)
	{
	}

	TuningSocketListener::~TuningSocketListener()
	{
	}

	void TuningSocketListener::onThreadStarted()
	{
		createSocket();
		startTimer();
	}

	void TuningSocketListener::onThreadFinished()
	{
		m_timer.stop();
		closeSocket();
	}

	void TuningSocketListener::createSocket()
	{
		if (m_socket != nullptr)
		{
			assert(false);
			return;
		}

		m_socket = new QUdpSocket(this);

		bool bindResult = m_socket->bind(m_listenIP.address(), m_listenIP.port());

		if (bindResult == true)
		{
			// successful binding
			//
			connect(m_socket, &QUdpSocket::readyRead, this, &TuningSocketListener::onSocketReadyRead);

			DEBUG_LOG_MSG(m_logger, QString(tr("Tuning listening socket is created and bound to %1")).arg(m_listenIP.addressPortStr()));
		}
		else
		{
			DEBUG_LOG_ERR(m_logger, QString(tr("Tuning listening socket error binding to %1")).arg(m_listenIP.addressPortStr()));

			// error binding
			//
			closeSocket();
		}
	}

	void TuningSocketListener::closeSocket()
	{
		if (m_socket == nullptr)
		{
			return;
		}

		m_socket->close();
		delete m_socket;
		m_socket = nullptr;
	}

	void TuningSocketListener::startTimer()
	{
		connect(&m_timer, &QTimer::timeout, this, &TuningSocketListener::onTimer);

		// start 1000 ms periodic timer
		//
		m_timer.setInterval(1000);
		m_timer.start();
	}

	void TuningSocketListener::onTimer()
	{
		if (m_socket == nullptr)
		{
			createSocket();
		}
	}

	void TuningSocketListener::onSocketReadyRead()
	{
		if (m_socket == nullptr)
		{
			assert(false);
			return;
		}

		QHostAddress tuningSourceIP;
		SimRupFotipV2 reply;

		int count = 0;

		while(m_socket->hasPendingDatagrams() && count < 1000)
		{
			count++;

			qint64 size = m_socket->pendingDatagramSize();

			if (size != sizeof(RupFotipV2) &&
				size != sizeof(SimRupFotipV2))
			{
				m_errReplySize++;

				// anyway read datagram but isn't process it
				//
				m_socket->readDatagram(reinterpret_cast<char*>(&reply), sizeof(reply), &tuningSourceIP);

				incSourceWorkerErrReplySize(tuningSourceIP);
				continue;
			}

			size = m_socket->readDatagram(reinterpret_cast<char*>(&reply), sizeof(reply), &tuningSourceIP);

			if (size == -1)
			{
				m_errReadSocket++;

				closeSocket();
				return;
			}

			if (size == sizeof(SimRupFotipV2))
			{
				// this is simulator datagram
				//
				if (m_simMode == false)
				{
					m_errNotExpectedSimPacket++;

					if ((m_errNotExpectedSimPacket % 300) == 0)
					{
						qDebug() << C_STR(QString("Software is not in SIMULATION mode, %1 sim packets has been ignored.").
										  arg(m_errNotExpectedSimPacket));
					}

					continue;
				}

				quint16 simVersion = reverseUint16(reply.simVersion);

				if (simVersion != 1)
				{
					m_errSimVersion++;
					continue;
				}

				// replace tuningSourceIP
				//
				tuningSourceIP.setAddress(reverseUint32(reply.tuningSourceIP));
			}
			else
			{
				if (size != sizeof(RupFotipV2))
				{
					continue;			// unknown datagramm size
				}

				// this is real LM datagram
			}

			pushReplyToTuningSourceWorker(tuningSourceIP, reply.rupFotipV2);
		}
	}

	void TuningSocketListener::pushReplyToTuningSourceWorker(const QHostAddress& tuningSourceIP, const RupFotipV2& reply)
	{
		quint32 sourceIP = tuningSourceIP.toIPv4Address();

		TuningSourceThread* sourceThread = m_sourceThreadMap.value(sourceIP, nullptr);

		if (sourceThread == nullptr)
		{
			m_errUnknownTuningSource++;

			DEBUG_LOG_ERR(m_logger, QString(tr("Reply from unknown tuning source %1")).
						  arg(tuningSourceIP.toString()));
			return;
		}

		sourceThread->pushReply(reply);
	}

	void TuningSocketListener::incSourceWorkerErrReplySize(const QHostAddress& tuningSourceIP)
	{
		quint32 sourceIP = tuningSourceIP.toIPv4Address();

		TuningSourceThread* sourceThread = m_sourceThreadMap.value(sourceIP, nullptr);

		if (sourceThread == nullptr)
		{
			m_errUnknownTuningSource++;
			return;
		}

		sourceThread->incErrReplySize();
	}

	// ----------------------------------------------------------------------------------
	//
	// TuningSocketListenerThread class implementation
	//
	// ----------------------------------------------------------------------------------

	TuningSocketListenerThread::TuningSocketListenerThread(const HostAddressPort& listenIP,
														   TuningSourceThreadMap& sourceWorkerMap,
														   bool simulationMode,
														   std::shared_ptr<CircularLogger> logger)
	{
		m_socketListener = new TuningSocketListener(listenIP, sourceWorkerMap, simulationMode, logger);

		addWorker(m_socketListener);
	}

	TuningSocketListenerThread::~TuningSocketListenerThread()
	{
	}

}

