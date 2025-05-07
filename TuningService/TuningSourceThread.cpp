#include <algorithm>

#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/Crc.h"
#include "../OnlineLib/CircularLogger.h"

#include <HardwareLib/DataProtocols.h>

#include "TuningSourceThread.h"
#include "TuningService.h"

namespace Tuning
{
	// ----------------------------------------------------------------------------------
	//
	// TuningSourceState struct implementation
	//
	// ----------------------------------------------------------------------------------

	void TuningSourceState::saveToProto(Network::TuningSourceState* tss) const
	{
		TEST_PTR_RETURN(tss);

		tss->set_sourceid(sourceID);
		tss->set_lanequipmentid(lanEquipmentID);

		//

		tss->set_isreply(isReply);
		tss->set_requestcount(requestCount);
		tss->set_replycount(replyCount);
		tss->set_commandqueuesize(commandQueueSize);
		tss->set_controlisactive(controlIsActive);
		tss->set_setsor(setSOR);
		tss->set_writingdisabled(writingDisabled);
		tss->set_hasunappliedparams(hasUnappliedParams);

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
		tss->set_fotipprocessingnumerator(fotipProcessingNumerator);

		//

		tss->set_errrupprotocolversion(errRupProtocolVersion);
		tss->set_errrupframesize(errRupFrameSize);
		tss->set_errrupnontuningdata(errRupNonTuningData);
		tss->set_errrupmoduletype(errRupModuleType);
		tss->set_errrupframesquantity(errRupFramesQuantity);
		tss->set_errrupframenumber(errRupFrameNumber);
		tss->set_errrupcrc(errRupCRC);

		//

		tss->set_errfotipprotocolversion(errFotipProtocolVersion);
		tss->set_errfotipuniqueid(errFotipUniqueID);
		tss->set_errfotiplmnumber(errFotipLmNumber);
		tss->set_errfotipsubsystemcode(errFotipSubsystemCode);
		tss->set_errfotipoperationcode(errFotipOperationCode);
		tss->set_errfotipframesize(errFotipFrameSize);
		tss->set_errfotipromsize(errFotipRomSize);
		tss->set_errfotipromframesize(errFotipRomFrameSize);
		tss->set_erranaloglowboundcheck(errAnalogLowBoundCheck);
		tss->set_erranaloghighboundcheck(errAnalogHighBoundCheck);

		//

		tss->set_erruntimelyreplay(errUntimelyReplay);
		tss->set_errsent(errSent);
		tss->set_errpartialsent(errPartialSent);
		tss->set_errreplysize(errReplySize);
		tss->set_errnoreply(errNoReply);
		tss->set_errtuningframeupdate(errTuningFrameUpdate);

		//

		tss->set_lmtime(lmTime);
	}

	// ----------------------------------------------------------------------------------
	//
	// SafeTuningValue class implementation
	//
	// ----------------------------------------------------------------------------------

	SafeTuningValue::SafeTuningValue()
	{
	}

	SafeTuningValue::SafeTuningValue(const SafeTuningValue& stv)
	{
		m_mutex.lock();

		m_value.setValue(stv.m_value);

		m_mutex.unlock();
	}

	SafeTuningValue& SafeTuningValue::operator = (const TuningValue& tv)
	{
		m_mutex.lock();

		m_value = tv;

		m_mutex.unlock();

		return *this;
	}

	bool SafeTuningValue::operator == (const SafeTuningValue& stv) const
	{
		bool res = false;

		m_mutex.lock();

		res = (m_value == stv.m_value);

		m_mutex.unlock();

		return res;
	}

	bool SafeTuningValue::operator == (const TuningValue& tv) const
	{
		bool res = false;

		m_mutex.lock();

		res = (m_value == tv);

		m_mutex.unlock();

		return res;
	}

	TuningValueType SafeTuningValue::type() const
	{
		TuningValueType t;

		m_mutex.lock();

		t = m_value.type();

		m_mutex.unlock();

		return t;
	}

	void SafeTuningValue::setType(TuningValueType t)
	{
		m_mutex.lock();

		m_value.setType(t);

		m_mutex.unlock();
	}

	TuningValue SafeTuningValue::tuningValue() const
	{
		TuningValue tv;

		m_mutex.lock();

		tv = m_value;

		m_mutex.unlock();

		return tv;
	}


	// ----------------------------------------------------------------------------------
	//
	// TuningCommand struct implementation
	//
	// ----------------------------------------------------------------------------------

	std::atomic<quint64> TuningCommand::m_globalCommandID = 0;

	TuningCommand::TuningCommand()
	{
		m_commandID = ++m_globalCommandID;
	}

	// ----------------------------------------------------------------------------------
	//
	// TuningCommandQueue class implementation
	//
	// ----------------------------------------------------------------------------------

	void TuningCommandQueue::push(const TuningCommand& cmd)
	{
		m_mutex.lock();

		m_queue.push(cmd);

		m_mutex.unlock();
	}

	bool TuningCommandQueue::pop(TuningCommand* cmd)
	{
		TEST_PTR_RETURN_FALSE(cmd);

		bool result = false;

		m_mutex.lock();

		if (m_queue.empty() == false)
		{
			*cmd = m_queue.front();
			m_queue.pop();
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

	TuningChannelHandler::TuningChannelHandler(TuningSourceThreadWorker& srcThread,
											   int rupVersion,
											   int fotipVersion,
												const TuningChannelInfo& channelInfo,
												bool disableModulesTypeChecking,
												E::SoftwareRunMode swRunMode,
												CircularLoggerShared logger,
												CircularLoggerShared tuningLog) :
		m_sourceThread(srcThread),
		m_rupVersion(rupVersion),
		m_fotipVersion(fotipVersion),
		m_channel(channelInfo.channel),
		m_logger(logger),
		m_tuningLog(tuningLog),
		m_socket(nullptr),
		m_replyQueue(5)
{
		Q_ASSERT(m_channel >=0 && m_channel < TuningServiceSettings::CHANNELS_COUNT);

		m_isSimulationMode = swRunMode == E::SoftwareRunMode::Simulation;

		const TuningSource& source = m_sourceThread.source();

		m_moduleEquipmentID = source.moduleEquipmentID();
		m_portEquipmentID = channelInfo.portEquipmentID;
		m_sourceIP = channelInfo.tuningDataIP;
		m_sourceUniqueID = source.moduleUniqueID();
		m_lmNumber = static_cast<quint16>(source.lmNumber());
		m_lmModuleType = static_cast<quint16>(source.moduleType());
		m_subsystemCode = static_cast<quint16>(source.subsystemKey());

		m_rupNumerator = static_cast<quint16>(m_channel * 1000);		// different numerator start value for each channel
		m_fotipRequestNumerator = m_rupNumerator;

		m_disableModulesTypeChecking = disableModulesTypeChecking;

		m_tuningSimIP = channelInfo.tuningSimIP;

		auto td = source.tuningData();

		TEST_PTR_RETURN(td);

		m_tuningFlashSizeB = td->tuningFlashFrameCount() * td->tuningFlashFrameSizeB();
		m_tuningFlashFramePayloadB = td->tuningFlashFramePayloadB();

		m_tuningDataOffsetW = td->tuningDataOffsetW();
		m_tuningDataFrameCount = td->tuningDataFrameCount();
		m_tuningDataFramePayloadW = td->tuningDataFramePayloadW();

		m_tuningUsedFramesCount = td->usedFramesCount();

		//

		m_state.sourceID = srcThread.source().ID();
		m_state.channel = m_channel;
		m_state.lanEquipmentID = channelInfo.portEquipmentID.toStdString();

		//

		if (m_channel > CHANNEL_1)
		{
			// set different frameNo to auto read for channel greater than CHANNEL_1
			//
			m_nextFrameToAutoRead = m_tuningUsedFramesCount / 2;
		}
		else
		{
			m_nextFrameToAutoRead = 0;
		}
	}

	TuningChannelHandler::~TuningChannelHandler()
	{
	}

	HostAddressPort TuningChannelHandler::sourceIP() const
	{
		return m_sourceIP;
	}

	QString TuningChannelHandler::sourceEquipmentID() const
	{
		return m_portEquipmentID;
	}

	int TuningChannelHandler::channel() const
	{
		return m_channel;
	}

	void TuningChannelHandler::startHandler()
	{
		m_state.controlIsActive = true;

		DEBUG_LOG_MSG(m_logger, QString("Tuning source %1 (%2, channel %3)  handler is started").
					  arg(m_portEquipmentID).
					  arg(m_sourceIP.addressPortStr()).
					  arg(m_channel + 1));

		m_isInitialized = true;
	}

	void TuningChannelHandler::stopHandler()
	{
		m_state.controlIsActive = false;

		DEBUG_LOG_MSG(m_logger, QString("Tuning source %1 (%2) handler is stopped").arg(m_portEquipmentID).arg(m_sourceIP.addressPortStr()));
	}

	void TuningChannelHandler::run()
	{
		//
		// TuningSourceThread call this function every 1 ms
		//
		if (m_waitReply == true)
		{
			bool replyReceived = processWaitReply();

			if (replyReceived == false)
			{
				return;			// nothing to do while not receive reply or reply timout elapsed
			}
		}

		processUntimelyReply();

		if (processCommandQueue() == true)
		{
			return;
		}

		enqueueTuningReadCommand();
	}

	bool TuningChannelHandler::isInitialized() const
	{
		return m_isInitialized;
	}

	bool TuningChannelHandler::isReply() const
	{
		return m_state.isReply;
	}

	bool TuningChannelHandler::setSOR() const
	{
		return m_state.setSOR;
	}

	bool TuningChannelHandler::writingDisabled() const
	{
		return m_state.writingDisabled;
	}

	void TuningChannelHandler::pushReply(const RupFotip& reply)
	{
		m_replyQueue.push(reply, QThread::currentThread());
	}

	void TuningChannelHandler::incErrReplySize()
	{
		m_state.errReplySize++;
	}

	void TuningChannelHandler::getState(Network::TuningSourceState* tuningSourceState)
	{
		TEST_PTR_RETURN(tuningSourceState);

		m_state.saveToProto(tuningSourceState);
	}

	void TuningChannelHandler::stopCommandProcessing(const TuningCommand& cmd, int srcChannel, bool hasUnappliedParams)
	{
		quint64 commandID = cmd.commandID();

		Q_UNUSED(srcChannel);

		if (m_alreadyProcessedCommands.size() == 1000)
		{
			auto next200 = std::next(m_alreadyProcessedCommands.begin(), 200);
			m_alreadyProcessedCommands.erase(m_alreadyProcessedCommands.begin(),
											 next200);	 // remove first 200 elements
		}

		auto it = m_alreadyProcessedCommands.find(commandID);

		if (it == m_alreadyProcessedCommands.end())
		{
			m_alreadyProcessedCommands.insert(commandID);
		}

		//

		m_state.hasUnappliedParams = hasUnappliedParams;
	}

	bool TuningChannelHandler::processWaitReply()
	{
		if (m_waitReply == false)
		{
			Q_ASSERT(false);
			return true;				// like as reply has been received
		}

		bool replyReceived = false;

		for(;;)
		{
			replyReceived = m_replyQueue.pop(&m_reply, QThread::currentThread());

			if (replyReceived == true && m_fotipVersion >= Fotip::V3)
			{
				quint64 replyNumerator = reverseUint64(m_reply.fotipFrame.header.requestNumerator);

				if (replyNumerator != m_request.rupFotip.fotipFrame.header.requestNumerator)
				{
					continue;		// skip reply with non expected requestNumerator
				}
			}

			break;
		}

		if (replyReceived == true)
		{

			m_state.isReply = true;
			m_state.replyCount++;

			//

			m_lastReplyTime = QDateTime::currentMSecsSinceEpoch();

			//

			Rup::TimeStamp rts = m_reply.rupHeader.timeStamp;

			rts.reverseBytes();

			m_state.lmTime = QDateTime(	QDate(rts.year, rts.month, rts.day),
										QTime(rts.hour, rts.minute, rts.second, rts.millisecond),
										TIME_ZONE_UTC).toMSecsSinceEpoch();

			//

			auto it = m_alreadyProcessedCommands.find(m_lastProcessedCommand.commandID());

			if (it == m_alreadyProcessedCommands.end())
			{
				processReply(m_reply);
			}
			else
			{
				m_alreadyProcessedCommands.erase(it);
			}

			m_waitReply = false;
			m_lastRequestTime = 0;
			m_retryCount = 0;

			//

			if ((m_state.replyCount % 100) == 0)
			{
				qDebug() << C_STR(QString("Receive %1 replies from %2, NoReplies = %3").
								  arg(m_state.replyCount).arg(m_portEquipmentID).arg(m_state.errNoReply));
			}

			return true;				// reply received
		}

		// reply isn't received yet

		if ((QDateTime::currentMSecsSinceEpoch() - m_lastRequestTime) < REPLY_TIMEOUT_MS)
		{
			return false;
		}

		m_lastRequestTime = 0;

		// fix replay timeout
		//
		m_state.errNoReply++;

		LOG_MSG(m_tuningLog, QString("%1 TIMEOUT on request to %2 (%3)").
									arg(toHex(m_request.rupFotip.rupHeader.numerator)).
									arg(m_portEquipmentID).
									arg(m_sourceIP.addressPortStr()));

		qDebug() << C_STR(QString("NoReply from %1 (%2) [RUP frame No = %3]").
						  arg(m_portEquipmentID).
						  arg(m_sourceIP.addressPortStr()).
						  arg(m_request.rupFotip.rupHeader.numerator));

		m_retryCount++;

		if (m_retryCount < MAX_RETRY_COUNT)
		{
			m_waitReply = false;

			// retry last request
			//
			sendFotipRequest(m_request, m_requestAppSignalID, true);

			return false;
		}

		onNoReply();

		m_state.isReply = false;
		m_waitReply = false;
		m_state.lmTime = 0;
		m_lastReplyTime = QDateTime::currentMSecsSinceEpoch();

		return true;				// reply isn't recived but TRUE returned to run other processings
	}

	void TuningChannelHandler::processUntimelyReply()
	{
		if (m_waitReply == false)
		{
			bool replyReceived = false;

			do
			{
				replyReceived = m_replyQueue.pop(&m_reply, QThread::currentThread());

				if (replyReceived == true)
				{
					m_state.errUntimelyReplay++;
					LOG_MSG(m_tuningLog, QString("???? UNTIMELY reply from %1").
							arg(m_portEquipmentID));
					m_lastReplyTime = QDateTime::currentMSecsSinceEpoch();
				}
			}
			while(replyReceived == true);	// m_replyQueue clearing
		}
	}

	bool TuningChannelHandler::processCommandQueue()
	{
		if (m_waitReply == true)
		{
			Q_ASSERT(false);
			return true;		// while wating reply has not another processing
		}

		if (QDateTime::currentMSecsSinceEpoch() - m_lastReplyTime < PAUSE_BEFORE_NEXT_REQUEST_MS)
		{
			return true;
		}

		// get command from queue and send FOTIP request
		//

		TuningCommand newCommand;

		if (m_tuningCommandQueue.pop(&newCommand) == false)
		{
			return false;		// queue is empty, go to next processing
		}

		// lazy clearing of m_alreadyProcessedCommands set

		if (m_alreadyProcessedCommands.size() > 0 && newCommand.commandID() > 50)
		{
			//
			// removing from m_alreadyProcessedCommands commandIDs that "distance" from new commandID more than 50
			//
			quint64 cmdIdToDelete = newCommand.commandID() - 50;

			for(int deletedCtr = 0; deletedCtr < 100; deletedCtr++)
			{
				auto first = m_alreadyProcessedCommands.begin();

				if (first == m_alreadyProcessedCommands.end())
				{
					break;
				}

				if (*first >= cmdIdToDelete)
				{
					break;
				}

				m_alreadyProcessedCommands.erase(first);
			}
		}

		//

		auto it = m_alreadyProcessedCommands.find(newCommand.commandID());

		if (it != m_alreadyProcessedCommands.end())
		{
			// command is already processed by another handler
			// skip command
			//
			m_alreadyProcessedCommands.erase(it);

			return true;
		}

		m_lastProcessedCommand = newCommand;

		bool result = prepareFotipRequest(m_lastProcessedCommand, m_request.rupFotip);

		if (result == false)
		{
			return false;
		}

		m_requestAppSignalID.clear();

		logTuningRequest(m_lastProcessedCommand, &m_requestAppSignalID, m_request.rupFotip.rupHeader.numerator);

		m_retryCount = 0;

		sendFotipRequest(m_request, m_requestAppSignalID, false);

		return true;
	}

	bool TuningChannelHandler::enqueueTuningReadCommand()
	{
		if (m_waitReply == true)
		{
			Q_ASSERT(false);
			return true;		// while wating reply has not another processing
		}

		TuningCommand tuningCmd;

		tuningCmd.opCode = Fotip::OpCode::Read;
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

	void TuningChannelHandler::onNoReply()
	{
		finalizeWriting(E::NetworkError::TuningNoReply);
	}

	bool TuningChannelHandler::prepareFotipRequest(const TuningCommand& tuningCmd, RupFotip &request)
	{
		bool result = true;

		result &= initRupHeader(request.rupHeader);

		result &= initFotipFrame(request.fotipFrame, tuningCmd);

		return result;
	}

	void TuningChannelHandler::sendFotipRequest(SimRupFotip& request, const QString& appSignalID, bool retry)
	{
		Q_ASSERT(sizeof(Rup::Frame) == Socket::ENTIRE_UDP_SIZE);
		Q_ASSERT(sizeof(RupFotip) == Socket::ENTIRE_UDP_SIZE);
		Q_ASSERT(sizeof(Fotip::Frame) == Rup::FRAME_DATA_SIZE);
		Q_ASSERT(sizeof(Fotip::Header) == 128);

		Q_ASSERT(m_waitReply == false);

		RupFotip& rupFotip = request.rupFotip;

		if (retry == true && m_fotipVersion >= Fotip::V3)
		{
			m_fotipRequestNumerator++;
			rupFotip.fotipFrame.header.requestNumerator = m_fotipRequestNumerator;
		}

		// convert headers to BigEndian
		//
		rupFotip.rupHeader.reverseBytes();
		rupFotip.fotipFrame.header.reverseBytes();

		//

		rupFotip.calcCRC64();

		qint64 sent = 0;

		if (m_isSimulationMode == false)
		{
			// packet sending to real LM
			//
			sent = m_socket.writeDatagram(reinterpret_cast<char*>(&rupFotip),
										  sizeof(rupFotip),
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

		m_state.requestCount++;

		// revert headers to LittleEndian
		//
		rupFotip.rupHeader.reverseBytes();
		rupFotip.fotipFrame.header.reverseBytes();

		//

		m_sourceThread.service().logTuningPacket(true,
								  static_cast<Fotip::OpCode>(rupFotip.fotipFrame.header.operationCode),
								  rupFotip.rupHeader.numerator,
								  rupFotip.fotipFrame.header.requestNumerator);
		//

		quint32 rawDiscreteValue = rupFotip.fotipFrame.write.discreteValue;
		quint32 rawBitmask = rupFotip.fotipFrame.write.bitMask;
		quint16 requestID = rupFotip.rupHeader.numerator;

		//

		m_lastRequestTime = QDateTime::currentMSecsSinceEpoch();

		m_waitReply = true;

		if (sent == -1)
		{
			m_state.errSent++;
			return;
		}

		if (sent < static_cast<qint64>(sizeof(m_request)))
		{
			m_state.errPartialSent++;
		}

		// logging
		//
		switch(static_cast<Fotip::OpCode>(rupFotip.fotipFrame.header.operationCode))
		{
		case Fotip::OpCode::Write:
			{
				QString valueStr = rupFotip.fotipFrame.valueStr(true);

				if (rupFotip.fotipFrame.isDiscreteData() == true)
				{
					DEBUG_LOG_MSG(m_logger, QString("%1 RupFotip WRITE request is sent to %2 (%3), signal %4 value %5."
													"StartAddrW %6, OffsetInFrameW %7, RawValue32 %8 BE, Bitmask32 %9 BE").
								  arg(toHex(requestID)).
								  arg(sourceEquipmentID()).
								  arg(m_sourceIP.addressStr()).
								  arg(appSignalID).
								  arg(valueStr).
								  arg(rupFotip.fotipFrame.header.startAddressW).
								  arg(rupFotip.fotipFrame.header.offsetInFrameW).
								  arg(rawDiscreteValue, 8, 16, QLatin1Char('0')).
								  arg(rawBitmask, 8, 16, QLatin1Char('0')));
				}
				else
				{
					DEBUG_LOG_MSG(m_logger, QString("%1 RupFotip WRITE request is sent to %2 (%3), signal %4 value %5").
								  arg(toHex(requestID)).
								  arg(sourceEquipmentID()).
								  arg(m_sourceIP.addressStr()).
								  arg(appSignalID).
								  arg(valueStr));
				}
			}
			break;

		case Fotip::OpCode::Apply:
			DEBUG_LOG_MSG(m_logger, QString("%1 RupFotip APPLY request is sent to %2 (%3)").
						  arg(toHex(requestID)).
						  arg(sourceEquipmentID()).
						  arg(m_sourceIP.addressStr()));
			break;

		case Fotip::OpCode::Read:
/*			DEBUG_LOG_MSG(m_logger, QString("%1 RupFotip READ request is sent to %2 (%3)").
						  arg(toHex(requestID)).
						  arg(sourceEquipmentID()).
						  arg(m_sourceIP.addressStr()));*/
			break;

		default:
			assert(false);
		}
	}

	bool TuningChannelHandler::initRupHeader(Rup::Header& rupHeader)
	{
		m_rupNumerator++;

		rupHeader.frameSize = Socket::ENTIRE_UDP_SIZE;
		rupHeader.protocolVersion = static_cast<quint16>(m_rupVersion);

		rupHeader.flags.all = 0;
		rupHeader.flags.tuningData = 1;

		rupHeader.dataId = 0;
		rupHeader.moduleType = m_lmModuleType;
		rupHeader.numerator = m_rupNumerator;
		rupHeader.framesQuantity = 1;
		rupHeader.frameNumber = 0;

		rupHeader.timeStamp.setDateTime(QDateTime::currentDateTime());

		return true;
	}

	bool TuningChannelHandler::initFotipFrame(Fotip::Frame& fotipFrame, const TuningCommand& tuningCmd)
	{
		if (m_fotipVersion >= Fotip::V3)
		{
			m_fotipRequestNumerator++;
		}

		Fotip::Header& fotipHeader = fotipFrame.header;

		// common initialization
		//
		fotipHeader.protocolVersion = static_cast<quint16>(m_fotipVersion);
		fotipHeader.uniqueId = m_sourceUniqueID;

		fotipHeader.subsystemKey.wordVaue = 0;
		fotipHeader.subsystemKey.lmNumber = m_lmNumber;
		fotipHeader.subsystemKey.subsystemCode = m_subsystemCode;
		fotipHeader.subsystemKey.crc = Crc::crc4(fotipHeader.subsystemKey.wordVaue);

		fotipHeader.flags.all = 0;

		fotipHeader.fotipFrameSizeB = sizeof(Fotip::Frame);

		fotipHeader.romSizeB = static_cast<quint32>(m_tuningFlashSizeB);
		fotipHeader.romFrameSizeB = static_cast<quint16>(m_tuningFlashFramePayloadB);

		fotipHeader.offsetInFrameW = 0;

		fotipHeader.requestNumerator = m_fotipRequestNumerator;		// from v3 of protocol

		fotipHeader.fotipProcessingNumerator = 0xEFCDAB8967452301l;					// from v3 of protocol

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
		case Fotip::OpCode::Read:
			fotipHeader.startAddressW = m_tuningDataOffsetW + tuningCmd.read.frame * m_tuningDataFramePayloadW;
			fotipHeader.dataType = TO_INT(Fotip::DataType::Discrete);		// any data type is allowed
			break;

		case Fotip::OpCode::Write:
			{
				TuningSignalShared ts = getTuningSignal(tuningCmd.write.signalHash);

				TEST_PTR_RETURN_FALSE(ts);

				int offsetW = ts->offset();

				int frameNo =  offsetW / m_tuningDataFramePayloadW;

				if ((frameNo % 3) != 0)
				{
					assert(false);
					return false;
				}

				fotipHeader.dataType = static_cast<quint16>(ts->fotipDataType());

				fotipHeader.startAddressW = m_tuningDataOffsetW + frameNo * m_tuningDataFramePayloadW;
				fotipHeader.offsetInFrameW = offsetW - frameNo * m_tuningDataFramePayloadW;

				fotipFrame.write.bitMask = 0;
				fotipFrame.write.discreteValue = 0;		// zero fotipFrame.write.floatValue also

				switch(ts->tuningValueType())
				{
				case TuningValueType::Float:
					fotipFrame.write.analogFloatValue = reverseFloat(tuningCmd.write.newTuningValue.floatValue());
					break;

				case TuningValueType::SignedInt32:
					fotipFrame.write.analogSignedIntValue = reverseInt32(tuningCmd.write.newTuningValue.int32Value());
					break;

				case TuningValueType::Discrete:
					{
						int bit = ts->bit();

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

				ts->initWriting(tuningCmd.commandID(),
								tuningCmd.clientEquipmentID,
								QDateTime::currentMSecsSinceEpoch());
			}
			break;

		case Fotip::OpCode::Apply:
			break;

		default:
			assert(false);
			return false;
		}

		return true;
	}

	void TuningChannelHandler::processReply(RupFotip& reply)
	{
		bool result = true;

		result = reply.checkCRC64();

		if (result == false)
		{
			finalizeWriting(E::NetworkError::TuningNoReply);
			m_state.errRupCRC++;
			return;
		}

		reply.rupHeader.reverseBytes();
		reply.fotipFrame.header.reverseBytes();

		result = checkRupHeader(reply.rupHeader);

		if (result == false)
		{
			finalizeWriting(E::NetworkError::TuningNoReply);
			return;
		}

		Fotip::Header& fotipHeader = reply.fotipFrame.header;

		result = checkFotipHeader(fotipHeader);

		if (result == false)
		{
			finalizeWriting(E::NetworkError::TuningNoReply);
			return;
		}

		Fotip::OpCode commandOpCode = static_cast<Fotip::OpCode>(fotipHeader.operationCode);

		switch(commandOpCode)
		{
		case Fotip::OpCode::Read:
			processReadReply(reply);
			break;

		case Fotip::OpCode::Write:
			result = processWriteReply(reply);
			break;

		case Fotip::OpCode::Apply:
			result = processApplyReply(reply);
			break;

		default:
			assert(false);
		}

		if (commandOpCode != Fotip::OpCode::Read && result == true)
		{
			// Write or Apply command is successfully processed
			// Stop processing of this command in other handlers
			//
			m_sourceThread.stopCommandProcessing(m_lastProcessedCommand, m_channel, m_state.hasUnappliedParams);
		}
	}

	bool TuningChannelHandler::processReadReply(RupFotip& reply)
	{
/*		QString msg;

		msg = QString("%1 Reply (%2ms, %3) received from %4 (%5) on RupFotip READ request.").
						arg(toHex(m_request.rupFotip.rupHeader.numerator)).
						arg(m_lastReplyTime - m_lastRequestTime).
						arg(toHex(reply.rupHeader.numerator)).
						arg(sourceEquipmentID()).
						arg(m_sourceIP.addressStr());

		DEBUG_LOG_MSG(m_logger, msg);*/

		bool res = m_sourceThread.updateFrameSignalsState(reply);

		if (res == false)
		{
			m_state.errTuningFrameUpdate++;
		}

		return res;
	}

	bool TuningChannelHandler::processWriteReply(RupFotip& reply)
	{
		bool res = m_sourceThread.updateFrameSignalsState(reply);

		if (res == false)
		{
			m_state.errTuningFrameUpdate++;
		}

		reply.fotipFrame.analogCmpErrors.all = reverseUint16(reply.fotipFrame.analogCmpErrors.all);

		QString msg;

		bool hasErrors = false;

		E::NetworkError errCode = E::NetworkError::Success;

		switch(static_cast<Fotip::DataType>(reply.fotipFrame.header.dataType))
		{
		case Fotip::DataType::AnalogFloat:
		case Fotip::DataType::AnalogSignedInt:
			{
				QString boundCheckStr;

				if (reply.fotipFrame.analogCmpErrors.highBoundCheckError == 1)
				{
					m_state.errAnalogHighBoundCheck++;

					boundCheckStr = QString("HighBoundCheckError == 1 ");
					errCode = E::NetworkError::TuningValueOutOfRange;
					hasErrors = true;
				}

				if (reply.fotipFrame.analogCmpErrors.lowBoundCheckError == 1)
				{
					m_state.errAnalogLowBoundCheck++;

					boundCheckStr = QString("LowBoundCheckError == 1 ");
					errCode = E::NetworkError::TuningValueOutOfRange;
					hasErrors = true;
				}

				if (reply.fotipFrame.analogCmpErrors.highBoundCheckError == 0 &&
					reply.fotipFrame.analogCmpErrors.lowBoundCheckError == 0)
				{
					boundCheckStr = ("No bound check errors ");
				}

				msg = QString("%1 Reply (%2) received from %3 (%4) on RupFotip WRITE request: %5").
								arg(toHex(m_request.rupFotip.rupHeader.numerator)).
								arg(toHex(reply.rupHeader.numerator)).
								arg(sourceEquipmentID()).
								arg(m_sourceIP.addressStr()).
								arg(boundCheckStr);
			}
			break;

		case Fotip::DataType::Discrete:
			{
				quint32 data32 = *reinterpret_cast<quint32*>(reply.fotipFrame.data + m_request.rupFotip.fotipFrame.header.offsetInFrameW * 2);

				msg = QString("%1 Reply (%2) received from %3 (%4) on RupFotip WRITE request. Data32[%5W] = %6").
								arg(toHex(m_request.rupFotip.rupHeader.numerator)).
								arg(toHex(reply.rupHeader.numerator)).
								arg(sourceEquipmentID()).
								arg(m_sourceIP.addressStr()).
								arg(m_request.rupFotip.fotipFrame.header.offsetInFrameW).
								arg(data32, 8, 16, QLatin1Char('0'));
			}
			break;

		default:
			Q_ASSERT(false);
		}

		TuningValue& newTuningValue = m_lastProcessedCommand.write.newTuningValue;

		TuningSignalShared ts = getTuningSignal(m_lastProcessedCommand.write.signalHash);

		if (ts != nullptr)
		{
			TuningValue currentValue = ts->currentTuningValueUnsafe();

			if (newTuningValue != currentValue)
			{
				errCode = E::NetworkError::TuningValueCorrupted;

				msg +=  QString("Tuning value corrupted");

				hasErrors = true;
			}
		}

		finalizeWriting(errCode);

		if (hasErrors == true)
		{
			DEBUG_LOG_ERR(m_logger, msg);
		}
		else
		{
			DEBUG_LOG_MSG(m_logger, msg);

			m_state.hasUnappliedParams = true;
		}

		logTuningReply(m_lastProcessedCommand, reply, m_request.rupFotip.rupHeader.numerator);

		return !hasErrors;
	}

	bool TuningChannelHandler::processApplyReply(RupFotip& reply)
	{
		bool res = true;

		QString result;

		if (reply.fotipFrame.header.flags.succesfulApply == 1)
		{
			result = "Success";
			m_state.hasUnappliedParams = false;
			res = true;
		}
		else
		{
			result = "Fail";
			res = false;
		}

		DEBUG_LOG_MSG(m_logger, QString("Reply is received from %1 (%2) on RupFotip APPLY request %3: %4").
					  arg(sourceEquipmentID()).
					  arg(m_sourceIP.addressStr()).
					  arg(reply.rupHeader.numerator, 4, 16, QLatin1Char('0')).
					  arg(result));

		logTuningReply(m_lastProcessedCommand, reply, m_request.rupFotip.rupHeader.numerator);

		return res;
	}

	void TuningChannelHandler::finalizeWriting(E::NetworkError errCode)
	{
		if (m_lastProcessedCommand.opCode != Fotip::OpCode::Write)
		{
			return;
		}

		TuningSignalShared ts = getTuningSignal(m_lastProcessedCommand.write.signalHash);

		TEST_PTR_RETURN(ts);

		ts->finalizeWriting(m_lastProcessedCommand.commandID(), errCode, QDateTime::currentMSecsSinceEpoch());
	}

	bool TuningChannelHandler::checkRupHeader(const Rup::Header& rupHeader)
	{
		bool result = true;

		if (rupHeader.protocolVersion != m_rupVersion)
		{
			m_state.errRupProtocolVersion++;
			result &= false;
		}

		if (rupHeader.frameSize != Socket::ENTIRE_UDP_SIZE)
		{
			m_state.errRupFrameSize++;
			result &= false;
		}

		if (rupHeader.timeStamp.isValid(false) == false)
		{
			m_state.errTimeStamp++;
			DEBUG_LOG_WRN(m_logger, QString("Error time stamp: %1").arg(rupHeader.timeStamp.rawToString(false)));
		}

		if (rupHeader.flags.tuningData != 1 ||
			rupHeader.flags.appData != 0 ||
			rupHeader.flags.diagData != 0 ||
			rupHeader.flags.test != 0)
		{
			m_state.errRupNonTuningData++;
			result &= false;
		}

		if (m_disableModulesTypeChecking == false && rupHeader.moduleType != m_lmModuleType)
		{
			m_state.errRupModuleType++;
			result &= false;

			DEBUG_LOG_ERR(m_logger, QString("Invalid moduleType of %1 (waiting %2, receiving %3)").
								arg(m_portEquipmentID).arg(m_lmModuleType).arg(rupHeader.moduleType));
		}

		if (rupHeader.framesQuantity != 1)
		{
			m_state.errRupFramesQuantity++;
			result &= false;
		}

		if (rupHeader.frameNumber != 0)
		{
			m_state.errRupFrameNumber++;
			result &= false;
		}

		//	quint32 dataId;	??

		return result;
	}

	bool TuningChannelHandler::checkFotipHeader(const Fotip::Header& fotipHeader)
	{
		bool result = true;

		if (fotipHeader.protocolVersion != m_fotipVersion)
		{
			m_state.errFotipProtocolVersion++;

			if ((m_state.errFotipProtocolVersion % 20) == 0)
			{
				DEBUG_LOG_ERR(m_logger, QString("%1 FOTIP version ERROR (version in request = %2, version in reply = %3)").
										arg(m_portEquipmentID).arg(m_fotipVersion).arg(fotipHeader.protocolVersion));
			}

			result = false;
		}

		if (fotipHeader.uniqueId != m_sourceUniqueID)
		{
			m_state.errFotipUniqueID++;

			if ((m_state.errFotipUniqueID % 500) == 0)
			{
				DEBUG_LOG_ERR(m_logger, QString("Wrong tuning source UniqueID: %1.").arg(m_sourceIP.addressStr()));
			}
			result = false;
		}
		else
		{
			m_state.errFotipUniqueID = 0;		// added by Vintenko 26.12.2017
		}

		if (fotipHeader.subsystemKey.lmNumber != m_lmNumber)
		{
			m_state.errFotipLmNumber++;
			result = false;
		}

		if (fotipHeader.subsystemKey.subsystemCode != m_subsystemCode)
		{
			m_state.errFotipSubsystemCode++;
			result = false;
		}

		if (fotipHeader.operationCode != m_request.rupFotip.fotipFrame.header.operationCode)
		{
			m_state.errFotipOperationCode++;
			result = false;
		}

		if (fotipHeader.fotipFrameSizeB != sizeof(Fotip::Frame))
		{
			m_state.errFotipFrameSize++;
			result = false;
		}

		if (fotipHeader.romSizeB !=  static_cast<quint32>(m_tuningFlashSizeB))
		{
			m_state.errFotipRomSize++;
			result = false;
		}

		if (fotipHeader.romFrameSizeB != m_tuningFlashFramePayloadB)
		{
			m_state.errFotipRomFrameSize++;
			result = false;
		}

		const Fotip::HeaderFlags& flags = fotipHeader.flags;

		// check FOTIP error flags
		//
		if (flags.dataTypeError == 1)
		{
			m_state.fotipFlagDataTypeErr++;
			result = false;
		}

		if (flags.operationCodeError == 1)
		{
			m_state.fotipFlagOpCodeErr++;
			result = false;
		}

		if (flags.startAddressError == 1)
		{
			m_state.fotipFlagStartAddrErr++;
			result = false;
		}

		if (flags.romSizeError == 1)
		{
			m_state.fotipFlagRomSizeErr++;
			result = false;
		}

		if (flags.romFrameSizeError == 1)
		{
			m_state.fotipFlagRomFrameSizeErr++;
			result = false;
		}

		if (flags.frameSizeError == 1)
		{
			m_state.fotipFlagFrameSizeErr++;
			result = false;
		}

		if (flags.versionError == 1)
		{
			m_state.fotipFlagProtocolVersionErr++;
			result = false;
		}

		if (flags.subsystemKeyError == 1)
		{
			m_state.fotipFlagSubsystemKeyErr++;
			result = false;
		}

		if (flags.idError == 1)
		{
			m_state.fotipFlagUniueIDErr++;
			result = false;
		}

		if (flags.offsetError == 1)
		{
			m_state.fotipFlagOffsetErr++;
			result = false;
		}

		// check FOTIP success flags
		//
		if (flags.successfulCheck == 1)
		{
			m_state.fotipFlagBoundsCheckSuccess++;
		}

		if (flags.successfulWrite == 1)
		{
			m_state.fotipFlagWriteSuccess++;
		}

		if (flags.succesfulApply == 1)
		{
			m_state.fotipFlagApplySuccess++;
		}

		if (flags.setSOR == 1)
		{
			m_state.fotipFlagSetSOR++;					// for platform LMs
			m_state.setSOR = true;

			m_state.fotipFlagWritingDisabled++;			// for non-platform LMs
			m_state.writingDisabled = true;
		}
		else
		{
			m_state.fotipFlagSetSOR = 0;					// for platform LMs
			m_state.setSOR = false;

			m_state.fotipFlagWritingDisabled = 0;		// for non-platform LMs
			m_state.writingDisabled = false;
		}

		return result;
	}

	void TuningChannelHandler::logTuningRequest(const TuningCommand& cmd, QString* appSignalID, quint16 requestNumerator)
	{
		TEST_PTR_RETURN(appSignalID);

		static const QString filler("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");

		QString logStr;

		switch(cmd.opCode)
		{
		case Fotip::OpCode::Read:
			return;

		case Fotip::OpCode::Write:
			{
				const TuningSignalShared ts = getTuningSignal(cmd.write.signalHash);

				TEST_PTR_RETURN(ts);

				logStr = QString("%1 WRITE request%2=> %3 Signal=%4 Type=%5 CurValue=%6 NewValue=%7 SOR=%8 (Client=%9, User=%10)").
							arg(toHex(requestNumerator)).
							arg(filler).
							arg(m_portEquipmentID).
							arg(ts->appSignalID()).
							arg(cmd.write.newTuningValue.typeStr()).
							arg(ts->currentTuningValueUnsafe().toString()).
							arg(cmd.write.newTuningValue.toString()).
							arg(m_state.setSOR == true ? 1 : 0).
							arg(cmd.clientEquipmentID).
							arg(cmd.matsUser);

				*appSignalID = ts->appSignalID();
			}
			break;

		case Fotip::OpCode::Apply:

			logStr = QString("%1 APPLY request%2=> %3 SOR=%4 (Client=%5, User=%6)").
								arg(toHex(requestNumerator)).
								arg(filler).
								arg(m_portEquipmentID).
								arg(m_state.setSOR == true ? 1 : 0).
								arg(cmd.clientEquipmentID).
								arg(cmd.matsUser);
			break;

		default:
			assert(false);
			return;
		}

		LOG_MSG(m_tuningLog, logStr);
	}

	void TuningChannelHandler::logTuningReply(const TuningCommand& cmd,
											  const RupFotip& reply,
											  quint16 requestNumerator)
	{
		QString logStr;

		switch(cmd.opCode)
		{
		case Fotip::OpCode::Read:
			{
				qint64 delay = m_lastReplyTime - m_lastRequestTime;

				if (delay >= 10)
				{
					logStr = QString("%1 READ reply (delay %2ms, %3)").
							arg(toHex(requestNumerator)).
							arg(delay).
							arg(toHex(reply.rupHeader.numerator));
				}
			}
			break;

		case Fotip::OpCode::Write:
			{
				const TuningSignalShared ts = getTuningSignal(cmd.write.signalHash);

				TEST_PTR_RETURN(ts);

				QString checkResultStr;

				if (ts->signalType() == E::SignalType::Analog)
				{
					checkResultStr = QString("LowBoundCheck=%1 HighBoundCheck=%2").
							arg(reply.fotipFrame.analogCmpErrors.lowBoundCheckError == 0 ? "Success" : "Fail").
							arg(reply.fotipFrame.analogCmpErrors.highBoundCheckError == 0 ? "Success" : "Fail");
				}

				logStr = QString("%1 WRITE reply (%2) <= %3 Signal=%4 CurValue=%5 %6 SOR=%7").
							arg(toHex(requestNumerator)).
							arg(toHex(reply.rupHeader.numerator)).
							arg(m_portEquipmentID).
							arg(ts->appSignalID()).
							arg(ts->currentTuningValueUnsafe().toString()).
							arg(checkResultStr).
							arg(reply.fotipFrame.header.flags.setSOR);
			}
			break;

		case Fotip::OpCode::Apply:
			logStr = QString("%1 APPLY reply (%2) <= %3 Result=%4 SOR=%5").
						arg(toHex(requestNumerator)).
						arg(toHex(reply.rupHeader.numerator)).
						arg(m_portEquipmentID).
						arg(reply.fotipFrame.header.flags.succesfulApply == 1 ? "Success" : "Fail").
						arg(m_state.setSOR == true ? 1 : 0);
			break;

		default:
			assert(false);
			return;
		}

		if (logStr.isEmpty() == false)
		{
			LOG_MSG(m_tuningLog, logStr);
		}
	}

	TuningSignalShared TuningChannelHandler::getTuningSignal(Hash signalHash)
	{
		return m_sourceThread.getTuningSignal(signalHash);
	}

	// ----------------------------------------------------------------------------------
	//
	// TuningSourceThread class implementation (QThread::run override)
	//
	// ----------------------------------------------------------------------------------

	TuningSourceThreadWorker::TuningSourceThreadWorker(TuningServiceWorker& service,
														const TuningServiceSettings& settings,
														const TuningSource& source,
														E::SoftwareRunMode swRunMode,
														CircularLoggerShared logger,
														CircularLoggerShared tuningLog) :
		m_service(service),
		m_source(source),
		m_swRunMode(swRunMode),
		m_logger(logger),
		m_tuningLog(tuningLog)
	{
		source.saveToProto(&m_protoDataSourceInfo);

		m_disableModulesTypeChecking = settings.disableModulesTypeChecking;

		m_tuningData = m_source.tuningData();

		if (m_tuningData == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		m_tuningDataOffsetW = m_tuningData->tuningDataOffsetW();
		m_tuningDataFramePayloadW = m_tuningData->tuningDataFramePayloadW();
		m_tuningDataFrameCount = m_tuningData->tuningDataFrameCount();

		m_tuningMem.init(m_tuningDataOffsetW, m_tuningDataFramePayloadW, m_tuningDataFrameCount);

		//

		for(int channel = CHANNEL_1; channel < TuningServiceSettings::CHANNELS_COUNT; channel++)
		{
			const TuningServiceSettings::ChannelSettings& ch = settings.channelSettings[channel];

			if (ch.enable == false)
			{
				continue;
			}

			TuningServiceSettings::TuningSource tsrc = ch.getTuningSource(m_source.moduleEquipmentID());

			if (tsrc.isValid() == false)
			{
				continue;
			}

			TuningChannelInfo tci;

			tci.portEquipmentID = tsrc.portEquipmentID;
			tci.channel = channel;
			tci.tuningDataIP = tsrc.tuningDataIP;
			tci.tuningSimIP = ch.tuningSimIP;

			m_tuningChannelsInfo.push_back(tci);
		}
	}

	void TuningSourceThreadWorker::onThreadStarted()
	{
		m_thread = QThread::currentThread();

		initTuningSignals();
		initHandlers();
		initTimer();
	}

	void TuningSourceThreadWorker::onThreadFinished()
	{
		shutdownTimer();
		shutdownHandlers();
	}

	void TuningSourceThreadWorker::timerEvent(QTimerEvent* event)
	{
		TEST_PTR_RETURN(event);
		TEST_PTR_RETURN(m_timer);

		if (event->timerId() == m_timer->timerId())
		{
			runHandlers();
			checkChannelsResponse();
			checkSetSOR();
		}
		else
		{
			QObject::timerEvent(event);
		}
	}

	void TuningSourceThreadWorker::pushReply(int channel, const RupFotip& reply)
	{
		AUTO_LOCK(m_handlersMutex);

		TuningChannelHandler* handler = getChannelHandler(channel);

		TEST_PTR_RETURN(handler);

		handler->pushReply(reply);
	}

	void TuningSourceThreadWorker::incErrReplySize(quint32 channelIP)
	{
		AUTO_LOCK(m_handlersMutex);

		auto it = m_ip2handlers.find(channelIP);

		if (it == m_ip2handlers.end())
		{
			return;
		}

		TuningChannelHandler* handler = it->second;

		TEST_PTR_RETURN(handler);

		handler->incErrReplySize();
	}

	void TuningSourceThreadWorker::getSourceState(Network::GetTuningSourcesStatesReply* reply) const
	{
		TEST_PTR_RETURN(reply);

		AUTO_LOCK(m_handlersMutex);

		for(auto handler : m_handlers)
		{
			TEST_PTR_CONTINUE(handler);

			Network::TuningSourceState* newTss = reply->add_tuningsourcesstate();

			TEST_PTR_CONTINUE(newTss);

			handler->state().saveToProto(newTss);
		}
	}

	void TuningSourceThreadWorker::getSourceState(Network::TuningSourceState* proto) const
	{
		TEST_PTR_RETURN(proto);

		AUTO_LOCK(m_handlersMutex);

		for(auto handler : m_handlers)
		{
			TEST_PTR_CONTINUE(handler);

			handler->state().saveToProto(proto);
		}
	}

	void TuningSourceThreadWorker::readSignalState(Network::TuningSignalState* tss) const
	{
		// this function called from another thread!!

		TEST_PTR_RETURN(tss);

		// tss->signalhash() is already filled
		//
		TuningSignalConstShared ts = getTuningSignal(tss->signalhash());

		if (ts == nullptr)
		{
			Q_ASSERT(false);			// how all previous checks we pass ???
			tss->set_valid(false);
			tss->set_error(TO_INT(E::NetworkError::UnknownSignalHash));
			return;
		}

		ts->saveToProto(tss, m_setSOR, m_writingDisabled, QThread::currentThread());
	}

	E::NetworkError TuningSourceThreadWorker::writeSignalState(const QString& clientEquipmentID,
														const QString& matsUser,
														Hash signalHash,
														const TuningValue& newValue)
	{
		const TuningSignalShared ts = getTuningSignal(signalHash);

		if (ts == nullptr)
		{
			Q_ASSERT(false);
			return E::NetworkError::UnknownSignalHash;
		}

		if (ts->tuningValueType() != newValue.type())
		{
			DEBUG_LOG_ERR(m_logger, QString("Tuning value type (%1) is not correspond to tuning signal %2 type (%3)").
											arg(newValue.typeStr()).
											arg(ts->appSignalID()).
											arg(ts->tuningValueTypeStr()));

			return E::NetworkError::WrongTuningValueType;
		}

		if (newValue < ts->lowBound() || newValue > ts->highBound())
		{
			DEBUG_LOG_ERR(m_logger, QString("New tuning value (%1) of tuning signal %2 is out of range (%3..%4)").
											arg(newValue.doubleValue()).
											arg(ts->appSignalID()).
											arg(ts->lowBound().toString()).
											arg(ts->highBound().toString()));

			return E::NetworkError::TuningValueOutOfRange;
		}

		TuningCommand cmd;

		cmd.clientEquipmentID = clientEquipmentID;
		cmd.matsUser = matsUser;

		cmd.opCode = Fotip::OpCode::Write;
		cmd.autoCommand = false;

		cmd.write.signalHash = signalHash;
		cmd.write.newTuningValue = newValue;

		pushCommandToHandlers(cmd, ts->appSignalID());

		return E::NetworkError::Success;
	}

	E::NetworkError TuningSourceThreadWorker::applySignalStates(const QString& clientEquipmentID,
														const QString& matsUser)
	{
		TuningCommand cmd;

		cmd.clientEquipmentID = clientEquipmentID;
		cmd.matsUser = matsUser;

		cmd.opCode = Fotip::OpCode::Apply;
		cmd.autoCommand = false;

		pushCommandToHandlers(cmd, QString());

		return E::NetworkError::Success;
	}

	QString TuningSourceThreadWorker::sourceEquipmentID() const
	{
		return m_source.moduleEquipmentID();
	}

	void TuningSourceThreadWorker::waitWhileHandlersInitialized() const
	{
		do
		{
			m_handlersMutex.lock();

			bool allInitialized = true;

			for(const TuningChannelHandler* handler : m_handlers)
			{
				allInitialized &= handler->isInitialized();
			}

			m_handlersMutex.unlock();

			if (allInitialized == true)
			{
				break;
			}

			QThread::msleep(1);
		}
		while(true);
	}

	bool TuningSourceThreadWorker::isSourceHandlerExistsForChannel(int channel) const
	{
		for(const TuningChannelInfo& tci : m_tuningChannelsInfo)
		{
			if (tci.channel == channel)
			{
				return true;
			}
		}

		return false;
	}

	TuningSignalConstShared TuningSourceThreadWorker::getTuningSignal(Hash hash) const
	{
		return std::const_pointer_cast<const TuningSignal>(privateGetTuningSignal(hash));
	}

	TuningSignalShared TuningSourceThreadWorker::getTuningSignal(Hash hash)
	{
		return privateGetTuningSignal(hash);
	}

	bool TuningSourceThreadWorker::updateFrameSignalsState(RupFotip& reply)
	{
		//
		// Byte order of reply.rupHeader already REVERSED (already in Little Endian)!!!
		//

		quint64 fotipProcessingNumerator = reply.fotipFrame.header.fotipProcessingNumerator;

		if ((fotipProcessingNumerator < m_fotipProcessingNumerator) &&
			(m_fotipProcessingNumerator - fotipProcessingNumerator) < 3)
		{
			// skip old data, this is not error!
			//
			return true;
		}

		m_fotipProcessingNumerator = fotipProcessingNumerator;

		bool updateResult = m_tuningMem.updateFrame(reply.fotipFrame.header.startAddressW,
													reply.fotipFrame.header.romFrameSizeB,
													reply.fotipFrame.data);
		if (updateResult == false)
		{
			return false;
		}

		// parse signals values and bounds
		//
		int frameNo = (reply.fotipFrame.header.startAddressW - m_tuningDataOffsetW) / m_tuningDataFramePayloadW;

		int arrayIndex = frameNo / 3;

		if (arrayIndex < 0 || arrayIndex >= std::ssize(m_frameSignals))
		{
			assert(false);
			return false;
		}

		const std::vector<int>& frameSignals = m_frameSignals[arrayIndex];

		quint8* dataPtr = reply.fotipFrame.data;

		qint64 updateTime = QDateTime::currentMSecsSinceEpoch();

		bool ok = false;

		qint64 lmTime = reply.rupHeader.timeStamp.toInt64(false, &ok);		// header already reversed!

		int tuningSignalsCount = TO_INT(m_tuningSignals.size());

		for(int signalIndex : frameSignals)
		{
			if (signalIndex < 0 || signalIndex >= tuningSignalsCount)
			{
				assert(false);
				continue;
			}

			TuningSignal& ts = *m_tuningSignals[signalIndex].get();

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
				{
					bool stateChanged = ts.setCurrentState(true, tuningValue, updateTime,
														  lmTime, m_fotipProcessingNumerator,
														  m_setSOR, m_writingDisabled);

					if (stateChanged == true)
					{
						m_service.pushSignalStateChange(ts.currentStateUnsafe(), m_thread);
					}
				}
				break;

			case 1:
				ts.setReadLowBound(tuningValue, m_setSOR, m_writingDisabled);
				break;

			case 2:
				ts.setReadHighBound(tuningValue, m_setSOR, m_writingDisabled);
				break;

			default:
				assert(false);
			}
		}

		return true;
	}

	void TuningSourceThreadWorker::stopCommandProcessing(const TuningCommand& cmd, int srcChannel, bool hasUnappliedParams)
	{
		for(TuningChannelHandler* handler : m_handlers)
		{
			TEST_PTR_CONTINUE(handler);

			if (handler->channel() != srcChannel)
			{
				handler->stopCommandProcessing(cmd, srcChannel, hasUnappliedParams);
			}
		}
	}

	void TuningSourceThreadWorker::initTuningSignals()
	{
		m_tuningSignals.clear();
		m_hash2SignalIndexMap.clear();
		m_frameSignals.clear();

		TuningDataSharedConst td = m_source.tuningData();

		TEST_PTR_RETURN(td);

		QVector<AppSignal*> tuningSignals;

		td->getSignals(&tuningSignals);

		int signalCount = static_cast<int>(tuningSignals.count());

		m_tuningSignals.resize(signalCount);

		for(int i = 0; i < signalCount; i++)
		{
			m_tuningSignals[i] = std::make_shared<TuningSignal>();

			AppSignal* signal = tuningSignals[i];

			TEST_PTR_CONTINUE(signal);

			Hash hash = calcHash(signal->appSignalID());

			if (m_hash2SignalIndexMap.contains(hash) == true)
			{
				Q_ASSERT(false);
				continue;
			}

			m_hash2SignalIndexMap.insert({hash, i});

			TuningSignal& ts = *m_tuningSignals[i].get();

			ts.init(signal, i, td->tuningDataFramePayloadW(), m_thread);

			int arrayIndex = ts.frameNo() / 3;

			Q_ASSERT(arrayIndex <= td->tuningDataFrameCount() / 3);

			while (arrayIndex >= std::ssize(m_frameSignals))			// appends new arrays if need
			{
				m_frameSignals.push_back(std::vector<int>());
			}

			m_frameSignals[arrayIndex].push_back(i);
		}
	}

	void TuningSourceThreadWorker::initHandlers()
	{
		AUTO_LOCK(m_handlersMutex);

		Q_ASSERT(m_handlers.size() == 0);

		for(const TuningChannelInfo& tci : m_tuningChannelsInfo)
		{
			TuningChannelHandler* handler = new TuningChannelHandler(*this,
																	 m_source.rupVersion(), m_source.fotipVersion(),
																	 tci, m_disableModulesTypeChecking,
																	 m_swRunMode, m_logger, m_tuningLog);

			m_handlers.push_back(handler);

			m_ch2handlers.insert({tci.channel, handler});
			m_ip2handlers.insert({tci.tuningDataIP.address32(), handler});

			handler->startHandler();
		}
	}

	void TuningSourceThreadWorker::shutdownHandlers()
	{
		AUTO_LOCK(m_handlersMutex);

		for(TuningChannelHandler* handler : m_handlers)
		{
			if (handler != nullptr)
			{
				handler->stopHandler();
				delete handler;
			}
			else
			{
				Q_ASSERT(false);
			}
		}

		m_handlers.clear();
	}

	void TuningSourceThreadWorker::initTimer()
	{
		Q_ASSERT(m_timer == nullptr);

		m_timer = new QBasicTimer();

		m_timer->start(1, Qt::PreciseTimer, this);
	}

	void TuningSourceThreadWorker::shutdownTimer()
	{
		TEST_PTR_RETURN(m_timer);

		m_timer->stop();

		delete m_timer;

		m_timer = nullptr;
	}

	const TuningChannelHandler* TuningSourceThreadWorker::getChannelHandler(int channel) const
	{
		return privateGetChannelHandler(channel);
	}

	TuningChannelHandler* TuningSourceThreadWorker::getChannelHandler(int channel)
	{
		return const_cast<TuningChannelHandler*>(privateGetChannelHandler(channel));
	}

	void TuningSourceThreadWorker::runHandlers()
	{
		for(TuningChannelHandler* handler : m_handlers)
		{
			handler->run();
		}
	}

	void TuningSourceThreadWorker::checkChannelsResponse()
	{
		bool anyChannelReply = false;

		for(TuningChannelHandler* handler : m_handlers)
		{
			anyChannelReply |= handler->isReply();
		}

		if (anyChannelReply == false && m_anyChannelReply == true)
		{
			invalidateAllSignals();
		}

		m_anyChannelReply = anyChannelReply;
	}

	void TuningSourceThreadWorker::invalidateAllSignals()
	{
		Q_ASSERT(QThread::currentThread() == m_thread);

		bool stateChanged = false;

		for(TuningSignalShared& s : m_tuningSignals)
		{
			stateChanged = s->invalidate();

			if (stateChanged == true)
			{
				m_service.pushSignalStateChange(s->currentStateUnsafe(), m_thread);
			}
		}
	}

	void TuningSourceThreadWorker::checkSetSOR()
	{
		bool setSOR = false;
		bool writingDisabled = false;

		for(TuningChannelHandler* handler : m_handlers)
		{
			if (handler->isReply() == true)
			{
				setSOR |= handler->setSOR();
				writingDisabled |= handler->writingDisabled();
			}
		}

		m_setSOR = setSOR;
		m_writingDisabled = writingDisabled;
	}

	void TuningSourceThreadWorker::pushCommandToHandlers(const TuningCommand& cmd, const QString& appSignalID)
	{
		AUTO_LOCK(m_handlersMutex);

		Q_UNUSED(appSignalID);

		for(TuningChannelHandler* handler : m_handlers)
		{
			handler->pushTuningCommand(cmd);

			switch(cmd.opCode)
			{
			case Fotip::OpCode::Read:
				break;

			case Fotip::OpCode::Write:
//				DEBUG_LOG_MSG(m_logger, QString("Enqueue WRITE command: source %1 channel %2 (%3), signal %4, value %5").
//							  arg(sourceEquipmentID()).
//							  arg(handler->channel() + 1).
//							  arg(handler->sourceIP().addressPortStr()).
//							  arg(appSignalID).
//							  arg(cmd.write.newTuningValue.toString()));
				break;

			case Fotip::OpCode::Apply:
//				DEBUG_LOG_MSG(m_logger, QString("Enqueue APPLY command: source %1 channel %2 (%3)").
//							  arg(sourceEquipmentID()).
//							  arg(handler->channel() + 1).
//							  arg(handler->sourceIP().addressPortStr()));
				break;

			default:
				Q_ASSERT(false);
			}
		}
	}

	const TuningChannelHandler* TuningSourceThreadWorker::privateGetChannelHandler(int channel) const
	{
		if (channel < 0 || channel >= TuningServiceSettings::CHANNELS_COUNT)
		{
			Q_ASSERT(false);
			return nullptr;
		}

		auto it = m_ch2handlers.find(channel);

		if (it == m_ch2handlers.end())
		{
			return nullptr;
		}

		return it->second;
	}

	TuningSignalShared TuningSourceThreadWorker::privateGetTuningSignal(Hash hash) const
	{
		auto it = m_hash2SignalIndexMap.find(hash);

		if (it == m_hash2SignalIndexMap.end())
		{
			Q_ASSERT(false);
			return nullptr;
		}

		int index = it->second;

		if (index < 0 || index >= std::ssize(m_tuningSignals))
		{
			Q_ASSERT(false);
			return nullptr;
		}

		return m_tuningSignals[index];
	}

	TuningSourceThread::TuningSourceThread(TuningServiceWorker& service,
											const TuningServiceSettings& settings,
											const TuningSource& source,
											E::SoftwareRunMode swRunMode,
											CircularLoggerShared logger,
											CircularLoggerShared tuningLog)
	{
		m_worker = new TuningSourceThreadWorker(service, settings, source,
												swRunMode, logger, tuningLog);
		addWorker(m_worker);
	}

	void TuningSourceThread::pushReply(int channel, const RupFotip& reply)
	{
		TEST_PTR_RETURN(m_worker);

		m_worker->pushReply(channel, reply);
	}

	void TuningSourceThread::incErrReplySize(quint32 channelIP)
	{
		TEST_PTR_RETURN(m_worker);

		m_worker->incErrReplySize(channelIP);
	}

	void TuningSourceThread::getSourceState(Network::GetTuningSourcesStatesReply* reply) const
	{
		TEST_PTR_RETURN(m_worker);

		m_worker->getSourceState(reply);
	}

	void TuningSourceThread::getSourceState(Network::TuningSourceState* proto) const
	{
		TEST_PTR_RETURN(m_worker);

		m_worker->getSourceState(proto);
	}

	void TuningSourceThread::readSignalState(Network::TuningSignalState* tss) const
	{
		TEST_PTR_RETURN(m_worker);

		m_worker->readSignalState(tss);
	}

	E::NetworkError TuningSourceThread::writeSignalState(const QString& clientEquipmentID,
														 const QString& matsUser,
														 Hash signalHash,
														 const TuningValue& newValue)
	{
		if (m_worker == nullptr)
		{
			Q_ASSERT(false);
			return E::NetworkError::InternalError;
		}

		return m_worker->writeSignalState(clientEquipmentID, matsUser, signalHash, newValue);
	}

	E::NetworkError TuningSourceThread::applySignalStates(	const QString& clientEquipmentID,
														const QString& user)
	{
		if (m_worker == nullptr)
		{
			Q_ASSERT(false);
			return E::NetworkError::InternalError;
		}

		return m_worker->applySignalStates(clientEquipmentID, user);
	}

	QString TuningSourceThread::sourceEquipmentID() const
	{
		if (m_worker == nullptr)
		{
			Q_ASSERT(false);
			return QString();
		}

		return m_worker->sourceEquipmentID();
	}

	void TuningSourceThread::waitWhileHandlersInitialized() const
	{
		TEST_PTR_RETURN(m_worker);

		m_worker->waitWhileHandlersInitialized();
	}

	bool TuningSourceThread::isSourceHandlerExistsForChannel(int channel) const
	{
		TEST_PTR_RETURN_FALSE(m_worker);

		return m_worker->isSourceHandlerExistsForChannel(channel);
	}

	// -------------------------------------------------------------------------
	//
	//	TuningSocketListener class implementaton
	//
	// -------------------------------------------------------------------------

	TuningSocketListener::TuningSocketListener(TuningServiceWorker& service,
											   const HostAddressPort& listenIP,
											   int channel,
											   bool simulationMode,
											   std::shared_ptr<CircularLogger> logger) :
		m_service(service),
		m_listenIP(listenIP),
		m_channel(channel),
		m_simMode(simulationMode),
		m_logger(logger)
	{
		Q_ASSERT(m_channel >= 0 && m_channel < TuningServiceSettings::CHANNELS_COUNT);
	}

	TuningSocketListener::~TuningSocketListener()
	{
	}

	void TuningSocketListener::onThreadStarted()
	{
		DEBUG_LOG_MSG(m_logger, QString(tr("Tuning channel %1 (IP %2) listening thread is started")).
					  arg(m_channel + 1).arg(m_listenIP.addressPortStr()));

		initTimer();
	}

	void TuningSocketListener::onThreadFinished()
	{
		shutdownTimer();

		closeSocket();

		DEBUG_LOG_MSG(m_logger, QString(tr("Tuning channel %1 (IP %2) listening thread finished")).
					  arg(m_channel + 1).arg(m_listenIP.addressPortStr()));
	}

	void TuningSocketListener::timerEvent(QTimerEvent* event)
	{
		TEST_PTR_RETURN(m_timer);

		if (event->timerId() != m_timer->timerId())
		{
			return;
		}

		if (m_socket == nullptr)
		{
			// Socket is not opened
			//
			createSocket();

			if (m_socket == nullptr)
			{
				return;
			}
		}

		// Socket is created
		//
		if (m_socket->hasPendingDatagrams() == true)
		{
			int count = 0;

			do
			{
				count++;

				bool result = readSocket();

				if (result == false)
				{
					break;
				}
			}
			while(m_socket->hasPendingDatagrams() == true && count < 500);
		}
	}

	void TuningSocketListener::initTimer()
	{
		Q_ASSERT(m_timer == nullptr);

		m_timer = new QBasicTimer();

		m_timer->start(1, Qt::PreciseTimer, this);
	}

	void TuningSocketListener::shutdownTimer()
	{
		TEST_PTR_RETURN(m_timer);

		m_timer->stop();

		delete m_timer;

		m_timer = nullptr;
	}

	void TuningSocketListener::createSocket()
	{
		if (m_socket != nullptr)
		{
			assert(false);
			return;
		}

		qint64 now = QDateTime::currentMSecsSinceEpoch();

		if (now - m_socketCreateLastTime < 1000)
		{
			return;
		}

		m_socketCreateLastTime = now;

		m_socket = new QUdpSocket();

		bool bindResult = m_socket->bind(m_listenIP.address(), m_listenIP.port());

		if (bindResult == true)
		{
			QVariant osRecvBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

			// successful binding
			//
			DEBUG_LOG_MSG(m_logger, QString(tr("Tuning channel %1 listening socket is created and bound to %2 (OS defined receive buffur size - %3 bytes))")).
						  arg(m_channel + 1).arg(m_listenIP.addressPortStr()).arg(osRecvBufSize.toInt()));

			if (osRecvBufSize.toInt() < 65536)
			{
				QVariant newRecvBufSize(static_cast<int>(65536));

				m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, newRecvBufSize);

				QVariant currentBufSize = m_socket->socketOption(QAbstractSocket::ReceiveBufferSizeSocketOption);

				DEBUG_LOG_MSG(m_logger, (QString("TuningSocketListenerThread: new receive buffer size is set - %1 bytes").arg(currentBufSize.toInt())));

				if (newRecvBufSize.toInt() != currentBufSize.toInt())
				{
					qDebug() << "";
					DEBUG_LOG_WRN(m_logger, QString("WARNING!!! Receive buffer size is not changed to required size."));
					DEBUG_LOG_MSG(m_logger, QString("Try change value of registry key (create if key is not exist)"));
					DEBUG_LOG_MSG(m_logger, QString("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\AFD\\Parameters\\DefaultReceiveWindow"));
					qDebug() << "";
				}
			}
		}
		else
		{
			DEBUG_LOG_ERR(m_logger, QString(tr("Tuning channel %1 listening socket error binding to %2")).
						  arg(m_channel + 1).
						  arg(m_listenIP.addressPortStr()));

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

	bool TuningSocketListener::readSocket()
	{
		if (m_socket == nullptr)
		{
			Q_ASSERT(false);
			return false;
		}

		QHostAddress tuningSourceIP;
		SimRupFotip reply;

		qint64 size = m_socket->pendingDatagramSize();

		if (size == -1)
		{
			closeSocket();			// why hasPendingDatagrams returns TRUE?
			return false;
		}

		if (size != sizeof(RupFotip) &&
			size != sizeof(SimRupFotip))
		{
			m_errReplySize++;

			// anyway read datagram but don't process it
			//
			m_socket->readDatagram(reinterpret_cast<char*>(&reply), sizeof(reply), &tuningSourceIP);

			incErrReplySizeOfTuningSource(tuningSourceIP);

			qDebug() << C_STR(QString("Wrong datagram size from %1. Reply rejected.").arg(tuningSourceIP.toString()));
			return true;
		}

		size = m_socket->readDatagram(reinterpret_cast<char*>(&reply), sizeof(reply), &tuningSourceIP);

		if (size == -1)
		{
			m_errReadSocket++;
			closeSocket();
			return false;
		}

		if (size == sizeof(SimRupFotip))
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

				return true;
			}

			quint16 simVersion = reverseUint16(reply.simVersion);

			if (simVersion != 1)
			{
				qDebug() << C_STR(QString("Simulation packet version %1 is not support").
								  arg(simVersion));

				m_errSimVersion++;

				return true;
			}

			// replace tuningSourceIP
			//
			tuningSourceIP.setAddress(reverseUint32(reply.tuningSourceIP));
		}

		//

		m_service.logTuningPacket(false,
								  static_cast<Fotip::OpCode>(reverseUint16(reply.rupFotip.fotipFrame.header.operationCode)),
								  reverseUint16(reply.rupFotip.rupHeader.numerator),
								  reverseUint64(reply.rupFotip.fotipFrame.header.requestNumerator));
		//

		pushReplyToTuningSource(tuningSourceIP, reply.rupFotip);

		return true;
	}

	void TuningSocketListener::pushReplyToTuningSource(const QHostAddress& tuningSourceIP, const RupFotip& reply)
	{
		quint32 sourceIP = tuningSourceIP.toIPv4Address();

		TuningSourceThreadShared sourceThread = m_service.getTuningSourceThread(sourceIP);

		if (sourceThread == nullptr)
		{
			m_errUnknownTuningSource++;

			DEBUG_LOG_ERR(m_logger, QString(tr("Reply from unknown tuning source %1")).
						  arg(tuningSourceIP.toString()));
			return;
		}

		sourceThread->pushReply(m_channel, reply);
	}

	void TuningSocketListener::incErrReplySizeOfTuningSource(const QHostAddress& tuningSourceIP)
	{
		quint32 sourceIP = tuningSourceIP.toIPv4Address();

		TuningSourceThreadShared sourceThread = m_service.getTuningSourceThread(sourceIP);

		if (sourceThread == nullptr)
		{
			m_errUnknownTuningSource++;
			return;
		}

		sourceThread->incErrReplySize(tuningSourceIP.toIPv4Address());
	}

	TuningSocketListenerThread::TuningSocketListenerThread(TuningServiceWorker& service,
												const HostAddressPort& listenIP,
												int channel,
												bool simulationMode,
												std::shared_ptr<CircularLogger> logger)
	{
		TuningSocketListener* worker = new TuningSocketListener(service, listenIP, channel,
																simulationMode, logger);
		addWorker(worker);
	}

}

