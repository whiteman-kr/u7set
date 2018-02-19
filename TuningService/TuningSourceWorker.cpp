#include "../lib/WUtils.h"
#include "../lib/Crc.h"
#include "../lib/CircularLogger.h"

#include "TuningSourceWorker.h"

namespace Tuning
{
	// ----------------------------------------------------------------------------------
	//
	// SourceStatistics struct implementation
	//
	// ----------------------------------------------------------------------------------

	void SourceStatistics::get(Network::TuningSourceState& tss)
	{
		tss.set_sourceid(dataSourceID);

		tss.set_isreply(isReply);

		tss.set_requestcount(requestCount);
		tss.set_replycount(replyCount);

		tss.set_commandqueuesize(commandQueueSize);

		tss.set_erruntimelyreplay(errUntimelyReplay);
		tss.set_errsent(errSent);
		tss.set_errpartialsent(errPartialSent);
		tss.set_errreplysize(errReplySize);
		tss.set_errnoreply(errNoReply);
		tss.set_errrupcrc(errRupCRC);

		// errors in reply RupFrameHeader
		//
		tss.set_errrupprotocolversion(errRupProtocolVersion);
		tss.set_errrupframesize(errRupFrameSize);
		tss.set_errrupnontuningdata(errRupNonTuningData);
		tss.set_errrupmoduletype(errRupModuleType);
		tss.set_errrupframesquantity(errRupFramesQuantity);
		tss.set_errrupframenumber(errRupFrameNumber);

		// errors in reply FotipHeader
		//
		tss.set_errfotipprotocolversion(errFotipProtocolVersion);
		tss.set_errfotipuniqueid(errFotipUniqueID);
		tss.set_errfotiplmnumber(errFotipLmNumber);
		tss.set_errfotipsubsystemcode(errFotipSubsystemCode);
		tss.set_errfotipoperationcode(errFotipOperationCode);
		tss.set_errfotipframesize(errFotipFrameSize);
		tss.set_errfotipromsize(errFotipRomSize);
		tss.set_errfotipromframesize(errFotipRomFrameSize);

		// errors reported by LM in reply FotipHeader.flags
		//
		tss.set_fotipflagboundschecksuccess(fotipFlagBoundsCheckSuccess);
		tss.set_fotipflagwritesuccess(fotipFlagWriteSuccess);
		tss.set_fotipflagdatatypeerr(fotipFlagDataTypeErr);
		tss.set_fotipflagopcodeerr(fotipFlagOpCodeErr);
		tss.set_fotipflagstartaddrerr(fotipFlagStartAddrErr);
		tss.set_fotipflagromsizeerr(fotipFlagRomSizeErr);
		tss.set_fotipflagromframesizeerr( fotipFlagRomFrameSizeErr);
		tss.set_fotipflagframesizeerr(fotipFlagFrameSizeErr);
		tss.set_fotipflagprotocolversionerr(fotipFlagProtocolVersionErr);
		tss.set_fotipflagsubsystemkeyerr(fotipFlagSubsystemKeyErr);
		tss.set_fotipflaguniueiderr(fotipFlagUniueIDErr);
		tss.set_fotipflagoffseterr(fotipFlagOffsetErr);
		tss.set_fotipflagapplysuccess(fotipFlagApplySuccess);
		tss.set_fotipflagsetsor(fotipFlagSetSOR);

		tss.set_erranaloglowboundcheck(errAnalogLowBoundCheck);
		tss.set_erranaloghighboundcheck(errAnalogHighBoundCheck);
	}

	// ----------------------------------------------------------------------------------
	//
	// TuningSourceWorker::TuningSignal class implementation
	//
	// ----------------------------------------------------------------------------------

	void TuningSourceWorker::TuningSignal::init(const Signal* s, int index, int tuningRomFraeSizeW)
	{
		if (s == nullptr)
		{
			assert(false);
			return;
		}

		m_valid = false;
		m_value = 0;

		m_signal = s;
		m_signalHash = ::calcHash(s->appSignalID());

		m_type = getTuningSignalType(s);
		m_index = index;
		m_lowBound = s->lowEngeneeringUnits();
		m_highBoud = s->highEngeneeringUnits();
		m_defaultValue = s->tuningDefaultValue();

		m_offset = s->tuningAddr().offset();
		m_bit = s->tuningAddr().bit();
		m_frameNo = s->tuningAddr().offset() / tuningRomFraeSizeW;
	}


	void TuningSourceWorker::TuningSignal::setState(bool valid, float value)
	{
		m_valid = valid;
		m_value = value;
	}


	void TuningSourceWorker::TuningSignal::setReadLowBound(float readLowBound)
	{
		m_readLowBound = readLowBound;
	}


	void TuningSourceWorker::TuningSignal::setReadHighBound(float readHighBound)
	{
		m_readHighBound = readHighBound;
	}


	QString TuningSourceWorker::TuningSignal::appSignalID() const
	{
		if (m_signal == nullptr)
		{
			assert(false);
			return "<Signal pointer == nullptr!>";
		}

		return m_signal->appSignalID();
	}


	FotipV2::DataType TuningSourceWorker::TuningSignal::getTuningSignalType(const Signal* s)
	{
		if (s == nullptr)
		{
			assert(false);
			return FotipV2::DataType::Discrete;
		}

		if (s->isAnalog() == true)
		{
			if (s->analogSignalFormat() == E::AnalogAppSignalFormat::Float32)
			{
				return FotipV2::DataType::AnalogFloat;
			}
			else
			{
				if (s->analogSignalFormat() == E::AnalogAppSignalFormat::SignedInt32)
				{
					return FotipV2::DataType::AnalogSignedInt;
				}
			}
		}
		else
		{
			if (s->isDiscrete() == true)
			{
				return FotipV2::DataType::Discrete;
			}
		}

		// unknown type of signal
		//
		assert(false);
		return FotipV2::DataType::Discrete;
	}


	// ----------------------------------------------------------------------------------
	//
	// TuningSourceWorker class implementation
	//
	// ----------------------------------------------------------------------------------

	TuningSourceWorker::TuningSourceWorker(const TuningServiceSettings& settings,
										   const TuningSource& source,
										   bool skipModuleTypeChecking,
										   std::shared_ptr<CircularLogger> logger) :
		m_logger(logger),
		m_skipModuleTypeChecking(skipModuleTypeChecking),
		m_timer(this),
		m_socket(this),
		m_replyQueue(this, 10),
		m_tuningCommandQueue(this, 1000)
	{
		m_sourceEquipmentID = source.lmEquipmentID();
		m_sourceIP = source.lmAddressPort();
		m_sourceUniqueID = source.uniqueID();
		m_lmNumber = static_cast<quint16>(source.lmNumber());
		m_lmModuleType = static_cast<quint16>(source.lmModuleType());
		m_subsystemCode = static_cast<quint16>(source.lmSubsystemID());

		m_tuningRomStartAddrW = settings.tuningDataOffsetW;
		m_tuningRomFrameCount = settings.tuningRomFrameCount;
		m_tuningRomFrameSizeW = settings.tuningRomFrameSizeW;
		m_tuningRomSizeW = settings.tuningRomSizeW;

		m_tuningMem.init(m_tuningRomStartAddrW, m_tuningRomFrameSizeW, m_tuningRomFrameCount);

		const TuningData* td = source.tuningData();

		if (td != nullptr)
		{
			m_tuningUsedFramesCount = td->usedFramesCount();
		}
		else
		{
			assert(false);
			m_tuningUsedFramesCount = m_tuningRomFrameCount;
		}

		m_stat.dataSourceID = source.ID();		// ID generated by DataSource::generateID()

		initTuningSignals(source.tuningData());
	}


	TuningSourceWorker::~TuningSourceWorker()
	{
	}


	quint32 TuningSourceWorker::sourceIP() const
	{
		return m_sourceIP.address32();
	}


	QString TuningSourceWorker::sourceEquipmentID() const
	{
		return m_sourceEquipmentID;
	}


	void TuningSourceWorker::pushReply(const Rup::Frame& reply)
	{
		m_replyQueue.push(&reply);
	}


	void TuningSourceWorker::incErrReplySize()
	{
		m_stat.errReplySize++;
	}


	void TuningSourceWorker::getState(Network::TuningSourceState& tuningSourceState)
	{
		m_stat.get(tuningSourceState);
	}


	void TuningSourceWorker::readSignalState(Network::TuningSignalState& tss)
	{
		// tss.signalhash() is already filled
		//
		Hash signalHash = tss.signalhash();

		int signalIndex = m_hash2SignalIndexMap.value(signalHash, -1);

		if (signalIndex == -1)
		{
			assert(false);			// how all previous checks we pass ???

			tss.set_valid(false);
			tss.set_error(TO_INT(NetworkError::UnknownSignalHash));
			return;
		}

		TuningSignal& signal = m_tuningSignals[signalIndex];

		tss.set_valid(signal.valid());
		tss.set_value(signal.value());
		tss.set_readlowbound(signal.readLowBound());
		tss.set_readhighbound(signal.readHighBound());
		tss.set_error(TO_INT(NetworkError::Success));
	}


	void TuningSourceWorker::writeSignalState(Hash signalHash, float value, Network::TuningSignalWriteResult& writeResult)
	{
		int signalIndex = m_hash2SignalIndexMap.value(signalHash, -1);

		if (signalIndex == -1)
		{
			writeResult.set_error(TO_INT(NetworkError::UnknownSignalHash));
			return;
		}

		if (signalIndex >= m_tuningSignals.count())
		{
			assert(false);
			DEBUG_LOG_ERR(m_logger, "Signal index out of range (TuningSourceWorker::writeSignalState)");
			return;
		}

		TuningCommand cmd;

		cmd.opCode = FotipV2::OpCode::Write;
		cmd.autoCommand = false;

		cmd.write.signalIndex = signalIndex;
		cmd.write.value = value;

		m_tuningCommandQueue.push(&cmd);

		writeResult.set_error(TO_INT(NetworkError::Success));

		DEBUG_LOG_MSG(m_logger, QString(tr("Queue write command: source %1 (%2), signal %3, value %4")).
					  arg(sourceEquipmentID()).
					  arg(m_sourceIP.addressStr()).
					  arg(m_tuningSignals[signalIndex].appSignalID()).
					  arg(value));
	}


	void TuningSourceWorker::applySignalStates()
	{
		TuningCommand cmd;

		cmd.opCode = FotipV2::OpCode::Apply;

		m_tuningCommandQueue.push(&cmd);

		DEBUG_LOG_MSG(m_logger, QString(tr("Queue apply command: source %1 (%2)")).
					  arg(sourceEquipmentID()).
					  arg(m_sourceIP.addressStr()));
	}


	void TuningSourceWorker::onThreadStarted()
	{
		connect(&m_replyQueue, &Queue<Rup::Frame>::queueNotEmpty, this, &TuningSourceWorker::onReplyReady);
		connect(&m_timer, &QTimer::timeout, this, &TuningSourceWorker::onTimer);

		m_timerInterval = 5;

		restartTimer();

		DEBUG_LOG_MSG(m_logger, QString(tr("Tuning source %1 worker is started")).arg(m_sourceIP.addressStr()));
	}


	void TuningSourceWorker::onThreadFinished()
	{
		DEBUG_LOG_MSG(m_logger, QString(tr("Tuning source %1 worker is finished")).arg(m_sourceIP.addressStr()));
	}


	void TuningSourceWorker::initTuningSignals(const TuningData* td)
	{
		m_tuningSignals.clear();
		m_hash2SignalIndexMap.clear();

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

			if (m_hash2SignalIndexMap.contains(hash))
			{
				assert(false);
				continue;
			}

			m_hash2SignalIndexMap.insert(hash, i);

			TuningSignal& ts = m_tuningSignals[i];

			ts.init(signal, i, m_tuningRomFrameSizeW);
		}
	}


	bool TuningSourceWorker::processWaitReply()
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

			onNoReply();

			m_retryCount++;

			if (m_retryCount >= MAX_RETRY_COUNT)
			{
				// fix - source is not reply
				//
				m_stat.isReply = false;

				invalidateAllSignals();

				qDebug() << "Invalidate signals on NoReply " << C_STR(m_sourceIP.addressStr());
			}
			else
			{
				// retry last request
				//
				sendFotipRequest(m_request);
			}
		}

		return false;			// switch to next processing
	}


	bool TuningSourceWorker::processCommandQueue()
	{
		if (m_waitReply == true)
		{
			return true;		// while wating reply has not another processing
		}

		if (m_tuningCommandQueue.isEmpty() == true)
		{
			return false;		// queue is empty, go to next processing
		}

		// get command from queue and send FOTIP request
		//

		TuningCommand tuningCmd;

		m_tuningCommandQueue.pop(&tuningCmd);

		bool result = prepareFotipRequest(tuningCmd, m_request);

		if (result == false)
		{
			return false;
		}

		m_retryCount = 0;

		sendFotipRequest(m_request);

		return true;
	}


	bool TuningSourceWorker::processIdle()
	{
		if (m_waitReply == true)
		{
			return true;		// while wating reply has not another processing
		}

		TuningCommand tuningCmd;

		tuningCmd.opCode = FotipV2::OpCode::Read;
		tuningCmd.read.frame = m_nextFrameToAutoRead;
		tuningCmd.autoCommand = true;

		m_tuningCommandQueue.push(&tuningCmd);

		m_nextFrameToAutoRead++;

		if (m_nextFrameToAutoRead >= m_tuningUsedFramesCount)
		{
			m_nextFrameToAutoRead = 0;
		}

		return false;
	}


	void TuningSourceWorker::onNoReply()
	{
	}


	bool TuningSourceWorker::prepareFotipRequest(const TuningCommand& tuningCmd, RupFotipV2& request)
	{
		bool result = true;

		result &= initRupHeader(request.rupHeader);

		result &= initFotipFrame(request.fotipFrame, tuningCmd);

		request.calcCRC64();

		return result;
	}


	void TuningSourceWorker::sendFotipRequest(RupFotipV2& request)
	{
		assert(sizeof(Rup::Frame) == ENTIRE_UDP_SIZE);
		assert(sizeof(RupFotipV2) == ENTIRE_UDP_SIZE);
		assert(sizeof(FotipV2::Frame) == Rup::FRAME_DATA_SIZE);
		assert(sizeof(FotipV2::Header) == 128);

		// convert headers to BigEndian
		//
		request.rupHeader.reverseBytes();
		request.fotipFrame.header.reverseBytes();

		quint64 sent = m_socket.writeDatagram(reinterpret_cast<char*>(&request),
											  sizeof(request),
											  m_sourceIP.address(),
											  m_sourceIP.port());
		// revert headers to LittleEndian
		//
		request.rupHeader.reverseBytes();
		request.fotipFrame.header.reverseBytes();

		m_waitReplyCounter = 0;

		m_waitReply = true;

		restartTimer();

		if (sent == -1)
		{
			m_stat.errSent++;
			return;
		}

		if (sent < sizeof(m_request))
		{
			m_stat.errPartialSent++;
		}

		// logging
		//
		switch(static_cast<FotipV2::OpCode>(request.fotipFrame.header.operationCode))
		{
		case FotipV2::OpCode::Write:
			{
				QString valueStr = request.fotipFrame.valueStr(true);

				DEBUG_LOG_MSG(m_logger, QString(tr("RupFotipV2 WRITE request is sent to %1 (%2), value %3")).
							  arg(sourceEquipmentID()).
							  arg(m_sourceIP.addressStr()).
							  arg(valueStr));
			}
			break;

		case FotipV2::OpCode::Apply:
			DEBUG_LOG_MSG(m_logger, QString(tr("RupFotipV2 APPLY request is sent to %1 (%2)")).
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


	bool TuningSourceWorker::initRupHeader(Rup::Header& rupHeader)
	{
		rupHeader.frameSize = ENTIRE_UDP_SIZE;
		rupHeader.protocolVersion = Rup::VERSION;

		rupHeader.flags.all = 0;
		rupHeader.flags.tuningData = 1;

		rupHeader.dataId = 0;
		rupHeader.moduleType = m_lmModuleType;
		rupHeader.numerator = m_rupNumerator;
		rupHeader.framesQuantity = 1;
		rupHeader.frameNumber = 0;

		QDateTime now = QDateTime::currentDateTime();

		QDate date = now.date();
		QTime time = now.time();

		rupHeader.timeStamp.year = date.year();
		rupHeader.timeStamp.month = date.month();
		rupHeader.timeStamp.day = date.day();

		rupHeader.timeStamp.hour = time.hour();
		rupHeader.timeStamp.minute = time.minute();
		rupHeader.timeStamp.second = time.second();
		rupHeader.timeStamp.millisecond = time.msec();

		m_rupNumerator++;

		return true;
	}


	bool TuningSourceWorker::initFotipFrame(FotipV2::Frame &fotipFrame, const TuningCommand& tuningCmd)
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

		fotipHeader.romSizeB = m_tuningRomSizeW * sizeof(quint16);										// in bytes
		fotipHeader.romFrameSizeB = static_cast<quint16>(m_tuningRomFrameSizeW * sizeof(quint16));		// in bytes

		fotipHeader.offsetInFrameW = 0;

		memset(fotipHeader.reserve, 0, sizeof(fotipHeader.reserve));

		fotipHeader.operationCode = TO_INT(tuningCmd.opCode);

		// operation-specific initialization
		//

		switch(tuningCmd.opCode)
		{
		case FotipV2::OpCode::Read:
			fotipHeader.startAddressW = m_tuningRomStartAddrW + tuningCmd.read.frame * m_tuningRomFrameSizeW;
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

				int frameNo =  offsetW / m_tuningRomFrameSizeW;

				if ((frameNo % 3) != 0)
				{
					assert(false);
					return false;
				}

				fotipHeader.dataType = TO_INT(ts.type());
				fotipHeader.startAddressW = m_tuningRomStartAddrW + frameNo * m_tuningRomFrameSizeW;
				fotipHeader.offsetInFrameW = offsetW - frameNo * m_tuningRomFrameSizeW;

				fotipFrame.write.bitMask = 0;
				fotipFrame.write.discreteValue = 0;		// zero fotipFrame.write.floatValue also

				switch(ts.type())
				{
				case FotipV2::DataType::AnalogFloat:
					fotipFrame.write.analogFloatValue = reverseFloat(static_cast<float>(tuningCmd.write.value));
					break;

				case FotipV2::DataType::AnalogSignedInt:
					fotipFrame.write.analogSignedIntValue = reverseInt32(static_cast<qint32>(tuningCmd.write.value));
					break;

				case FotipV2::DataType::Discrete:
					{
						int bit = ts.bit();

						assert(bit >= 0 && bit < 32 );

						quint32 bitmask = 1 << bit;

						//quint32 bitmask = ~0;

						fotipFrame.write.bitMask = reverseUint32(bitmask);

						quint32 discreteValue = (tuningCmd.write.value == 0.0 ? 0 : 1) << bit;

						//quint32 discreteValue = 0;

						fotipFrame.write.discreteValue = reverseUint32(discreteValue);
					}
					break;

				default:
					assert(false);
				}
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


	void TuningSourceWorker::processReply(RupFotipV2& reply)
	{
		bool result = true;

		result = reply.checkCRC64();

		if (result == false)
		{
			m_stat.errRupCRC++;
			return;
		}

		reply.rupHeader.reverseBytes();
		reply.fotipFrame.header.reverseBytes();

		result = checkRupHeader(reply.rupHeader);

		if (result == false)
		{
			return;
		}

		FotipV2::Header& fotipHeader = reply.fotipFrame.header;

		result = checkFotipHeader(fotipHeader);

		if (result == false)
		{
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


	void TuningSourceWorker::processReadReply(RupFotipV2& reply)
	{
		m_tuningMem.updateFrame(reply.fotipFrame.header.startAddressW,
								reply.fotipFrame.header.romFrameSizeB,
								reply.fotipFrame.data);

		// parse signals values and bounds
		//
		int frameNo = (reply.fotipFrame.header.startAddressW - m_tuningRomStartAddrW) / m_tuningRomFrameSizeW;

		quint8* dataPtr = reply.fotipFrame.data;

		int signalCount = m_tuningSignals.count();

		for(int i = 0; i < signalCount; i++)
		{
			TuningSignal& ts = m_tuningSignals[i];

			if (frameNo < ts.frameNo() ||
				frameNo > ts.frameNo() + 2)
			{
				continue;		// signal is not in this frame
			}

			float value = 0;

			int offsetInFrameB = (ts.offset() - ts.frameNo() * m_tuningRomFrameSizeW) * sizeof(quint16);

			assert(offsetInFrameB < reply.fotipFrame.header.romFrameSizeB);

			switch(ts.type())
			{
			case FotipV2::DataType::AnalogFloat:
				{
					value = reverseFloat(*reinterpret_cast<float*>(dataPtr + offsetInFrameB));
				}
				break;

			case FotipV2::DataType::AnalogSignedInt:
				{
					value = reverseInt32(*reinterpret_cast<qint32*>(dataPtr + offsetInFrameB));
				}
				break;

			case FotipV2::DataType::Discrete:
				{
					quint32 word =	reverseUint32(*reinterpret_cast<quint32*>(dataPtr + offsetInFrameB));
					value = ((word & (1 << ts.bit())) == 0 ? 0 : 1);
				}
				break;

			default:
				assert(false);
			}

			// (frameNo % 3) == 0 - tuning signal value
			// (frameNo % 3) == 1 - tuning signal read low bound
			// (frameNo % 3) == 2 - tuning signal read high bound

			switch(frameNo % 3)
			{
			case 0:
				ts.setState(true, value);
				break;

			case 1:
				ts.setReadLowBound(value);
				break;

			case 2:
				ts.setReadHighBound(value);
				break;

			default:
				assert(false);
			}
		}
	}


	void TuningSourceWorker::processWriteReply(RupFotipV2& reply)
	{
		reply.fotipFrame.analogCmpErrors.all = reverseUint16(reply.fotipFrame.analogCmpErrors.all);

		QString msg;
		bool hasErrors = false;

		switch(static_cast<FotipV2::DataType>(reply.fotipFrame.header.dataType))
		{
		case FotipV2::DataType::AnalogFloat:
		case FotipV2::DataType::AnalogSignedInt:
			{
				QString boundCheckStr("No bound check errors");

				if (reply.fotipFrame.analogCmpErrors.highBoundCheckError == 1)
				{
					m_stat.errAnalogHighBoundCheck++;

					boundCheckStr = QString("HighBoundCheckError == 1 ");
					hasErrors = true;
				}

				if (reply.fotipFrame.analogCmpErrors.lowBoundCheckError == 1)
				{
					m_stat.errAnalogLowBoundCheck++;

					boundCheckStr = QString("LowBoundCheckError == 1 ");
					hasErrors = true;
				}

				msg = QString(tr("Reply is received from %1 (%2) on RupFotipV2 WRITE request: %3")).
								arg(sourceEquipmentID()).
								arg(m_sourceIP.addressStr()).
								arg(boundCheckStr);
			}
			break;

		case FotipV2::DataType::Discrete:
			msg = QString(tr("Reply is received from %1 (%2) on RupFotipV2 WRITE request")).
							arg(sourceEquipmentID()).
							arg(m_sourceIP.addressStr());
			break;

		default:
			assert(false);
		}

		if (hasErrors == true)
		{
			DEBUG_LOG_ERR(m_logger, msg);
		}
		else
		{
			DEBUG_LOG_MSG(m_logger, msg);
		}
	}


	void TuningSourceWorker::processApplyReply(RupFotipV2&)
	{
		DEBUG_LOG_MSG(m_logger, QString(tr("Reply is received from %1 (%2) on RupFotipV2 APPLY request")).
					  arg(sourceEquipmentID()).
					  arg(m_sourceIP.addressStr()));
	}


	bool TuningSourceWorker::checkRupHeader(const Rup::Header& rupHeader)
	{
		bool result = true;

		if (rupHeader.protocolVersion != Rup::VERSION)
		{
			m_stat.errRupProtocolVersion++;
			result &= false;
		}

		if (rupHeader.frameSize != ENTIRE_UDP_SIZE)
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

		if (m_skipModuleTypeChecking == false && rupHeader.moduleType != m_lmModuleType)
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


	bool TuningSourceWorker::checkFotipHeader(const FotipV2::Header& fotipHeader)
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

		if (fotipHeader.operationCode != m_request.fotipFrame.header.operationCode)
		{
			m_stat.errFotipOperationCode++;
			result = false;
		}

		if (fotipHeader.fotipFrameSizeB != sizeof(FotipV2::Frame))
		{
			m_stat.errFotipFrameSize++;
			result = false;
		}

		if (fotipHeader.romSizeB !=  m_tuningRomSizeW * 2)
		{
			m_stat.errFotipRomSize++;
			result = false;
		}

		if (fotipHeader.romFrameSizeB != m_tuningRomFrameSizeW * 2)
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
			m_stat.fotipFlagSetSOR++;
		}
		else
		{
			m_stat.fotipFlagSetSOR = 0;	// added by Vintenko 22.12.2017
		}

		return result;
	}


	void TuningSourceWorker::restartTimer()
	{
		m_timer.start(m_timerInterval);
	}


	void TuningSourceWorker::invalidateAllSignals()
	{
		for(TuningSignal& s : m_tuningSignals)
		{
			s.setState(false, 0);
		}
	}


	void TuningSourceWorker::onTimer()
	{
		if (processWaitReply() == true)
		{
			return;
		}

		if (processCommandQueue() == true)
		{
			return;
		}

		processIdle();
	}


	void TuningSourceWorker::onReplyReady()
	{
		assert(sizeof(Rup::Frame) == sizeof(RupFotipV2));

		// convert reply from Rup::Frame to RupFotipV2
		//
		bool res = m_replyQueue.pop(reinterpret_cast<Rup::Frame*>(&m_reply));

		if (res == false)
		{
			return;
		}

		if (m_waitReply == false)
		{
			m_stat.errUntimelyReplay++;
			return;
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

		restartTimer();

		processReply(m_reply);

		m_waitReply = false;
	}


	// ----------------------------------------------------------------------------------
	//
	// TuningSourceWorkerThread class implementation
	//
	// ----------------------------------------------------------------------------------

	TuningSourceWorkerThread::TuningSourceWorkerThread(const TuningServiceSettings& settings,
													   const TuningSource& source,
													   bool skipModuleTypeChecking,
													   std::shared_ptr<CircularLogger> logger)
	{
		m_sourceWorker = new TuningSourceWorker(settings, source, skipModuleTypeChecking, logger);

		addWorker(m_sourceWorker);
	}


	TuningSourceWorkerThread::~TuningSourceWorkerThread()
	{
	}


	quint32 TuningSourceWorkerThread::sourceIP()
	{
		if (m_sourceWorker == nullptr)
		{
			assert(false);
			return 0;
		}

		return m_sourceWorker->sourceIP();
	}


	TuningSourceWorker* TuningSourceWorkerThread::worker()
	{
		return m_sourceWorker;
	}


	void TuningSourceWorkerThread::pushReply(const Rup::Frame& reply)
	{
		if (m_sourceWorker == nullptr)
		{
			assert(false);
			return;
		}

		m_sourceWorker->pushReply(reply);
	}


	void TuningSourceWorkerThread::incErrReplySize()
	{
		if (m_sourceWorker == nullptr)
		{
			assert(false);
			return;
		}

		m_sourceWorker->incErrReplySize();
	}


	// -------------------------------------------------------------------------
	//
	//	TuningSocketWorker class implementaton
	//
	// -------------------------------------------------------------------------

	TuningSocketListener::TuningSocketListener(const HostAddressPort &listenIP, \
											   TuningSourceWorkerThreadMap& sourceWorkerMap,
											   std::shared_ptr<CircularLogger> logger) :
		m_listenIP(listenIP),
		m_sourceWorkerMap(sourceWorkerMap),
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
			assert(false);
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
		Rup::Frame reply;

		int count = 0;

		while(m_socket->hasPendingDatagrams() && count < 1000)
		{
			count++;

			qint64 size = m_socket->pendingDatagramSize();

			if (size != sizeof(reply))
			{
				m_errReplySize++;

				// anyway read datagram but isn't process it
				//
				m_socket->readDatagram(reinterpret_cast<char*>(&reply), sizeof(reply), &tuningSourceIP);

				incSourceWorkerErrReplySize(tuningSourceIP);
				continue;
			}

			qint64 result = m_socket->readDatagram(reinterpret_cast<char*>(&reply), sizeof(reply), &tuningSourceIP);

			if (result == -1)
			{
				m_errReadSocket++;

				closeSocket();
				return;
			}

			pushReplyToTuningSourceWorker(tuningSourceIP, reply);
		}
	}


	void TuningSocketListener::pushReplyToTuningSourceWorker(const QHostAddress& tuningSourceIP, const Rup::Frame& reply)
	{
		quint32 sourceIP = tuningSourceIP.toIPv4Address();

		TuningSourceWorkerThread* sourceWorkerThread = m_sourceWorkerMap.value(sourceIP, nullptr);

		if (sourceWorkerThread == nullptr)
		{
			m_errUnknownTuningSource++;

			DEBUG_LOG_ERR(m_logger, QString(tr("Reply from unknown tuning source %1")).
						  arg(tuningSourceIP.toString()));
			return;
		}

		sourceWorkerThread->pushReply(reply);
	}


	void TuningSocketListener::incSourceWorkerErrReplySize(const QHostAddress& tuningSourceIP)
	{
		quint32 sourceIP = tuningSourceIP.toIPv4Address();

		TuningSourceWorkerThread* sourceWorkerThread = m_sourceWorkerMap.value(sourceIP, nullptr);

		if (sourceWorkerThread == nullptr)
		{
			m_errUnknownTuningSource++;
			return;
		}

		sourceWorkerThread->incErrReplySize();
	}


	// ----------------------------------------------------------------------------------
	//
	// TuningSocketListenerThread class implementation
	//
	// ----------------------------------------------------------------------------------

	TuningSocketListenerThread::TuningSocketListenerThread(const HostAddressPort& listenIP,
														   TuningSourceWorkerThreadMap& sourceWorkerMap,
														   std::shared_ptr<CircularLogger> logger)
	{
		m_socketListener = new TuningSocketListener(listenIP, sourceWorkerMap, logger);

		addWorker(m_socketListener);
	}

	TuningSocketListenerThread::~TuningSocketListenerThread()
	{
	}

}

