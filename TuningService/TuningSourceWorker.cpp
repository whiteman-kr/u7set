#include "../lib/WUtils.h"
#include "../lib/Crc.h"
#include "TuningSourceWorker.h"


namespace Tuning
{
	// ----------------------------------------------------------------------------------
	//
	// SourceStatistics struct implementation
	//
	// ----------------------------------------------------------------------------------

	void SourceStatistics::get(Network::TuningSourceState& tuningSourceState)
	{
	}

	// ----------------------------------------------------------------------------------
	//
	// TuningSourceWorker class implementation
	//
	// ----------------------------------------------------------------------------------

	TuningSourceWorker::TuningSourceWorker(const TuningServiceSettings& settings, const TuningSource& source) :
		m_timer(this),
		m_socket(this),
		m_tuningCommandQueue(this, 1000),
		m_replyQueue(this, 10)
	{
		m_sourceEquipmentID = source.lmEquipmentID();
		m_sourceIP = source.lmAddressPort();
		m_sourceUniqueID = source.uniqueID();
		m_lmNumber = static_cast<quint16>(source.lmNumber());
		m_subsystemCode = static_cast<quint16>(source.lmSubsystemID());

		m_tuningRomStartAddrW = settings.tuningDataOffsetW;
		m_tuningRomFrameCount = settings.tuningRomFrameCount;
		m_tuningRomFrameSizeW = settings.tuningRomFrameSizeW;
		m_tuningRomSizeW = settings.tuningRomSizeW;

		m_tuningMem.init(m_tuningRomStartAddrW, m_tuningRomFrameSizeW, m_tuningRomFrameCount);

		initTuningSignals(source.tuningData());
	}


	TuningSourceWorker::~TuningSourceWorker()
	{
	}


	quint32 TuningSourceWorker::sourceIP() const
	{
		return m_sourceIP.address32();
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


	void TuningSourceWorker::onThreadStarted()
	{
		connect(&m_replyQueue, &Queue<Rup::Frame>::queueNotEmpty, this, &TuningSourceWorker::onReplyReady);
		connect(&m_timer, &QTimer::timeout, this, &TuningSourceWorker::onTimer);

		m_timerInterval = 5;

		restartTimer();
	}


	void TuningSourceWorker::onThreadFinished()
	{
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

			ts.signal = signal;
			ts.offset = signal->tuningAddr().offset();
			ts.bit = signal->tuningAddr().bit();

			if (signal->isAnalog())
			{
				if (signal->analogSignalFormat() == E::AnalogAppSignalFormat::Float32)
				{
					ts.type = SignalType::AnalogFloat;
				}
				else
				{
					if (signal->analogSignalFormat() == E::AnalogAppSignalFormat::SignedInt32)
					{
						ts.type = SignalType::AnalogInt;
					}
					else
					{
						assert(false);		// unknown type of signal
					}
				}
			}
			else
			{
				if (signal->isDiscrete())
				{
					ts.type = SignalType::Discrete;
				}
				else
				{
					assert(false);		// unknown type of signal
				}
			}
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

			if ((m_stat.errNoReply % 100) == 0)
			{
				qDebug() << C_STR(QString(tr("From '%1' NoReplies = %2")).arg(m_sourceEquipmentID).arg(m_stat.errNoReply));
			}

			m_retryCount++;

			if (m_retryCount >= MAX_RETRY_COUNT)
			{
				// fix - source is not reply
				//
				m_stat.isReply = false;
			}
			else
			{
				// retry last request
				//
				sendFOTIPRequest(m_request);

//				qDebug() << "Retry last REQUEST";
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

		prepareFOTIPRequest(tuningCmd, m_request);

		m_retryCount = 0;

		sendFOTIPRequest(m_request);

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

		m_tuningCommandQueue.push(&tuningCmd);

		m_nextFrameToAutoRead++;

		if (m_nextFrameToAutoRead >= m_tuningRomFrameCount)
		{
			m_nextFrameToAutoRead = 0;
		}

		return false;
	}


	void TuningSourceWorker::prepareFOTIPRequest(const TuningCommand& tuningCmd, RupFotipV2& request)
	{
		initRupHeader(m_request.rupHeader);

		initFotipHeader(m_request.fotipFrame.header, tuningCmd);

		initFotipData(m_request.fotipFrame, tuningCmd);
	}


	void TuningSourceWorker::sendFOTIPRequest(RupFotipV2& request)
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

		if (sent == -1)
		{
			m_stat.errSent++;
			return;
		}

		if (sent < sizeof(m_request))
		{
			m_stat.errPartialSent++;
		}
	}


	void TuningSourceWorker::initRupHeader(Rup::Header& rupHeader)
	{
		rupHeader.frameSize = ENTIRE_UDP_SIZE;
		rupHeader.protocolVersion = Rup::VERSION;

		rupHeader.flags.all = 0;
		rupHeader.flags.tuningData = 1;

		rupHeader.dataId = 0;
		rupHeader.moduleType = Hardware::DeviceModule::FamilyType::LM;	// 0x1100
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
	}


	void TuningSourceWorker::initFotipHeader(FotipV2::Header& fotipHeader, const TuningCommand& tuningCmd)
	{
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

		fotipHeader.romSizeB = m_tuningRomSizeW * sizeof(quint16);				// in bytes
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

		default:
			assert(false);
		}
	}


	void TuningSourceWorker::initFotipData(FotipV2::Frame& fotip, const TuningCommand& tuningCmd)
	{
		switch(tuningCmd.opCode)
		{
		case FotipV2::OpCode::Read:
			break;

		default:
			assert(false);
		}
	}


	void TuningSourceWorker::processReply(RupFotipV2& reply)
	{
		reply.rupHeader.reverseBytes();
		reply.fotipFrame.header.reverseBytes();

		bool result = true;

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

		switch(fotipHeader.operationCode)
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
	}


	void TuningSourceWorker::processWriteReply(RupFotipV2& reply)
	{

	}


	void TuningSourceWorker::processApplyReply(RupFotipV2& reply)
	{

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
			m_stat.errRupNoTuningData++;
			result &= false;
		}

		if (rupHeader.moduleType != Hardware::DeviceModule::FamilyType::LM)
		{
			m_stat.errRupModuleType++;
			result &= false;
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
			result &= false;
		}

		if (fotipHeader.uniqueId != m_sourceUniqueID)
		{
			m_stat.errFotipUniqueID++;
			result &= false;
		}

		if (fotipHeader.subsystemKey.lmNumber != m_lmNumber)
		{
			m_stat.errFotipLmNumber++;
			result &= false;
		}

		if (fotipHeader.subsystemKey.subsystemCode != m_subsystemCode)
		{
			m_stat.errFotipSubsystemCode++;
			result &= false;
		}

		if (fotipHeader.operationCode != m_request.fotipFrame.header.operationCode)
		{
			m_stat.errFotipOperationCode++;
			result &= false;
		}

		if (fotipHeader.fotipFrameSizeB != sizeof(FotipV2::Frame))
		{
			m_stat.errFotipFrameSize++;
			result &= false;
		}

		if (fotipHeader.romSizeB !=  m_tuningRomSizeW * 2)
		{
			m_stat.errFotipRomSize++;
			result &= false;
		}

		if (fotipHeader.romFrameSizeB != m_tuningRomFrameSizeW * 2)
		{
			m_stat.errFotipRomFrameSize++;
			result &= false;
		}

		const FotipV2::HeaderFlags& flags = fotipHeader.flags;

		// check FOTIP error flags
		//
		if (flags.dataTypeError == 1)
		{
			m_stat.fotipFlagDataTypeErr++;
			result &= false;
		}

		if (flags.operationCodeError == 1)
		{
			m_stat.fotipFlagOpCodeErr++;
			result &= false;
		}

		if (flags.startAddressError == 1)
		{
			m_stat.fotipFlagStartAddrErr++;
			result &= false;
		}

		if (flags.romSizeError == 1)
		{
			m_stat.fotipFlagRomSizeErr++;
			result &= false;
		}

		if (flags.romFrameSizeError == 1)
		{
			m_stat.fotipFlagRomFrameSizeErr++;
			result &= false;
		}

		if (flags.frameSizeError == 1)
		{
			m_stat.fotipFlagFrameSizeErr++;
			result &= false;
		}

		if (flags.versionError == 1)
		{
			m_stat.fotipFlagProtocolVersionErr++;
			result &= false;
		}

		if (flags.subsystemKeyError == 1)
		{
			m_stat.fotipFlagSubsystemKeyErr++;
			result &= false;
		}

		if (flags.idError == 1)
		{
			m_stat.fotipFlagUniueIDErr++;
			result &= false;
		}

		if (flags.offsetError == 1)
		{
			m_stat.fotipFlagOffsetErr++;
			result &= false;
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

		return result;
	}


	void TuningSourceWorker::restartTimer()
	{
		m_timer.start(m_timerInterval);
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

		//restartTimer();

		processReply(m_reply);

		m_waitReply = false;
	}


	// ----------------------------------------------------------------------------------
	//
	// TuningSourceWorkerThread class implementation
	//
	// ----------------------------------------------------------------------------------

	TuningSourceWorkerThread::TuningSourceWorkerThread(const TuningServiceSettings& settings, const TuningSource& source)
	{
		m_sourceWorker = new TuningSourceWorker(settings, source);

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

	TuningSocketListener::TuningSocketListener(const HostAddressPort &listenIP, TuningSourceWorkerThreadMap& sourceWorkerMap) :
		m_listenIP(listenIP),
		m_sourceWorkerMap(sourceWorkerMap),
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

			qDebug() << "Tuning listen socket created and bound to:" << C_STR(m_listenIP.addressPortStr());
		}
		else
		{
			qDebug() << "Tuning listen error binding to:" << C_STR(m_listenIP.addressPortStr());

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
														   TuningSourceWorkerThreadMap& sourceWorkerMap)
	{
		m_socketListener = new TuningSocketListener(listenIP, sourceWorkerMap);

		addWorker(m_socketListener);
	}

	TuningSocketListenerThread::~TuningSocketListenerThread()
	{
	}

}

