#include "../lib/WUtils.h"
#include "../lib/Crc.h"
#include "TuningSourceWorker.h"


namespace Tuning
{

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
		m_sourceIP = source.lmAddressPort();
		m_sourceUniqueID = source.uniqueID();
		m_lmNumber = static_cast<quint16>(source.lmNumber());
		m_subsystemCode = static_cast<quint16>(source.lmSubsystemID());

		m_tuningRomStartAddr = settings.tuningDataOffsetW;
		m_tuningRomFrameCount = settings.tuningRomFrameCount;
		m_tuningRomFrameSizeW = settings.tuningRomFrameSizeW;
		m_tuningRomSizeW = settings.tuningRomSizeW;
	}


	quint32 TuningSourceWorker::sourceIP() const
	{
		return m_sourceIP.address32();
	}


	void TuningSourceWorker::pushReply(const RupFotipFrame& reply)
	{
		m_replyQueue.push(&reply);
	}


	void TuningSourceWorker::incErrReplySize()
	{
		m_errReplySize++;
	}


	void TuningSourceWorker::onThreadStarted()
	{
		connect(&m_replyQueue, &Queue<RupFotipFrame>::queueNotEmpty, this, &TuningSourceWorker::onReplyReady);
		connect(&m_timer, &QTimer::timeout, this, &TuningSourceWorker::onTimer);

		m_timer.setInterval(20);
		m_timer.start();
	}


	void TuningSourceWorker::onThreadFinished()
	{
	}


	bool TuningSourceWorker::processWaitReply()
	{
//		AUTO_LOCK(m_waitReplyMutex)

		if (m_waitReply == true)
		{
			m_waitReplyCounter++;

			if (m_waitReplyCounter < MAX_WAIT_REPLY_COUNTER)
			{
				return true;
			}

			// fix replay timeout
			//

			m_errNoReply++;

			m_waitReply = false;
		}

		return false;			// switch to next processing
	}


	bool TuningSourceWorker::processCommandQueue()
	{
//		AUTO_LOCK(m_waitReplyMutex)

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

		sendFOTIPRequest(tuningCmd);

		m_waitReply = true;

		return true;
	}


	bool TuningSourceWorker::processIdle()
	{
//		AUTO_LOCK(m_waitReplyMutex)

		if (m_waitReply == true)
		{
			return true;		// while wating reply has not another processing
		}

		TuningCommand tuningCmd;

		tuningCmd.opCode = OperationCode::Read;
		tuningCmd.read.frame = m_nextFrameToAutoRead;

		m_tuningCommandQueue.push(&tuningCmd);

		m_nextFrameToAutoRead++;

		if (m_nextFrameToAutoRead >= m_tuningRomFrameCount)
		{
			m_nextFrameToAutoRead = 0;
		}

		return false;
	}


	void TuningSourceWorker::sendFOTIPRequest(const TuningCommand& tuningCmd)
	{
		initRupHeader(m_request);

		initFotipHeader(m_request.fotip, tuningCmd);

		initFotipData(m_request.fotip, tuningCmd);

		quint64 sent = m_socket.writeDatagram(reinterpret_cast<char*>(&m_request),
											  sizeof(m_request),
											  m_sourceIP.address(),
											  m_sourceIP.port());
		if (sent == -1)
		{
			m_errSent++;
			return;
		}

		if (sent < sizeof(m_request))
		{
			m_errPartialSent++;
		}
	}


	void TuningSourceWorker::initRupHeader(RupFotipFrame& rup)
	{
		rup.rupHeader.frameSize = ENTIRE_UDP_SIZE;
		rup.rupHeader.protocolVersion = RUP_PROTOCOL_VERSION_5;

		rup.rupHeader.flags.all = 0;
		rup.rupHeader.flags.tuningData = 1;

		rup.rupHeader.dataId = 0;
		rup.rupHeader.moduleType = Hardware::DeviceModule::FamilyType::LM;	// 0x1100
		rup.rupHeader.numerator = m_rupNumerator;
		rup.rupHeader.framesQuantity = 1;
		rup.rupHeader.frameNumber = 0;

		QDateTime now = QDateTime::currentDateTime();

		QDate date = now.date();
		QTime time = now.time();

		rup.rupHeader.timeStamp.year = date.year();
		rup.rupHeader.timeStamp.month = date.month();
		rup.rupHeader.timeStamp.day = date.day();

		rup.rupHeader.timeStamp.hour = time.hour();
		rup.rupHeader.timeStamp.minute = time.minute();
		rup.rupHeader.timeStamp.second = time.second();
		rup.rupHeader.timeStamp.millisecond = time.msec();

		rup.rupHeader.reverseBytes();

		m_rupNumerator++;
	}


	void TuningSourceWorker::initFotipHeader(FotipFrame& fotip, const TuningCommand& tuningCmd)
	{
		// common initialization
		//
		fotip.header.protocolVersion = FOTIP_PROTOCOL_VERSION_2;
		fotip.header.uniqueId = m_sourceUniqueID;

		fotip.header.subsystemKey.wordVaue = 0;
		fotip.header.subsystemKey.lmNumber = m_lmNumber;
		fotip.header.subsystemKey.subsystemCode = m_subsystemCode;
		fotip.header.subsystemKey.crc = Crc::crc4(fotip.header.subsystemKey.wordVaue);

		fotip.header.flags.all = 0;

		fotip.header.fotipFrameSize = sizeof(FotipFrame);

		fotip.header.romSize = m_tuningRomSizeW * 2;				// in bytes
		fotip.header.romFrameSize = m_tuningRomFrameSizeW * 2;		// in bytes

		memset(fotip.header.reserve, 0, sizeof(fotip.header.reserve));

		fotip.header.operationCode = TO_INT(tuningCmd.opCode);

		// operation-specific initialization
		//

		switch(tuningCmd.opCode)
		{
		case OperationCode::Read:
			fotip.header.startAddress = m_tuningRomStartAddr + tuningCmd.read.frame * m_tuningRomFrameSizeW;
			fotip.header.dataType = 0;
			break;

		default:
			assert(false);
		}

		fotip.header.reverseBytes();
	}


	void TuningSourceWorker::initFotipData(FotipFrame& fotip, const TuningCommand& tuningCmd)
	{
		switch(tuningCmd.opCode)
		{
		case OperationCode::Read:
			break;

		default:
			assert(false);
		}
	}


	void TuningSourceWorker::processReply(RupFotipFrame& reply)
	{
		reply.rupHeader.reverseBytes();
		reply.fotip.header.reverseBytes();

		bool result = true;

		result = checkRupHeader(reply.rupHeader);

		if (result == false)
		{
			return;
		}

		result = checkFotipHeader(reply.fotip.header);

		if (result == false)
		{
			return;
		}



		int a = 0;
		a++;
	}


	bool TuningSourceWorker::checkRupHeader(const RupFrameHeader& rupHeader)
	{
		if (rupHeader.protocolVersion != RUP_PROTOCOL_VERSION_5)
		{
			m_errRupProtocolVersion++;
			return false;
		}

		if (rupHeader.frameSize != ENTIRE_UDP_SIZE)
		{
			m_errRupFrameSize++;
			return false;
		}

		if (rupHeader.flags.tuningData != 1 ||
			rupHeader.flags.appData != 0 ||
			rupHeader.flags.diagData != 0 ||
			rupHeader.flags.test != 0)
		{
			m_errRupNoTuningData++;
			return false;
		}

		if (rupHeader.moduleType != Hardware::DeviceModule::FamilyType::LM)
		{
			m_errRupModuleType++;
			return false;
		}

		if (rupHeader.framesQuantity != 1)
		{
			m_errRupFramesQuantity++;
			return false;
		}

		if (rupHeader.frameNumber != 0)
		{
			m_errRupFrameNumber++;
			return false;
		}

		//	quint32 dataId;	??

		return true;
	}


	bool TuningSourceWorker::checkFotipHeader(const FotipHeader& fotipHeader)
	{
		if (fotipHeader.protocolVersion != FOTIP_PROTOCOL_VERSION_2)
		{
			m_errFotipProtocolVersion++;
			return false;
		}

		if (fotipHeader.uniqueId != m_sourceUniqueID)
		{
			m_errFotipUniqueID++;
			return false;
		}

		if (fotipHeader.subsystemKey.lmNumber != m_lmNumber)
		{
			m_errFotipLmNumber++;
			return false;
		}

		if (fotipHeader.subsystemKey.subsystemCode != m_subsystemCode)
		{
			m_errFotipSubsystemCode++;
			return false;
		}

		if (fotipHeader.operationCode != m_request.fotip.header.operationCode)
		{
			m_errFotipOperationCode++;
			return false;
		}

		if (fotipHeader.fotipFrameSize != sizeof(FotipFrame))
		{
			m_errFotipFrameSize++;
			return false;
		}

		if (fotipHeader.romSize !=  m_tuningRomSizeW * 2)
		{
			m_errFotipRomSize++;
			return false;
		}

		if (fotipHeader.romFrameSize != m_tuningRomFrameSizeW * 2)
		{
			m_errFotipRomFrameSize++;
			return false;
		}

		FotipHeaderFlags& flags = fotipHeader.flags;

		if (flags.dataTypeError == 1)
		{
			m_errFotipFlagsDataType++;
			return false;
		}

		//		quint32 startAddress;	??
		//		quint16 dataType;		??

		return true;
	}


	void TuningSourceWorker::onTimer()
	{
		do
		{
			if (processWaitReply() == true) break;
			if (processCommandQueue() == true) break;

			processIdle();

			break;
		}
		while(1);
	}


	void TuningSourceWorker::onReplyReady()
	{
		bool res = m_replyQueue.pop(&m_reply);

		if (res == false)
		{
			return;
		}

		if (m_waitReply == false)
		{
			m_errUntimelyReplay++;
			return;
		}

		m_replyCount++;

		processReply(m_reply);

		m_waitReply = false;
	}



	/*void TuningSocketListener::onRequest(quint32 tuningSourceIP)
	{
		TuningSocketRequestQueue* queue = m_requestQueues.value(tuningSourceIP, nullptr);

		if (queue == nullptr)
		{
			assert(false);
			return;
		}

		if (queue->isWaitingForAck() || queue->isEmpty())
		{
			return;
		}

		TuningSocketRequest request;

		bool result = queue->pop(&request);

		if (result == false)
		{
			assert(false);
			return;
		}

		SocketRequest sr;

		m_requests.pop(&sr);

		if (m_socketBound == false)
		{
			return;
		}

		RupFrameHeader& rh = m_reqFrame.rupHeader;

		rh.frameSize = ENTIRE_UDP_SIZE;
		rh.protocolVersion = 4;
		rh.flags.all = 0;
		rh.flags.tuningData = 1;
		rh.dataId = 0;
		rh.moduleType = 4352;			// !!!!???
		rh.numerator = sr.numerator;

		rh.framesQuantity = 1;
		rh.frameNumber = 0;

		QDateTime t = QDateTime::currentDateTime();

		rh.timeStamp.year = t.date().year();
		rh.timeStamp.month = t.date().month();
		rh.timeStamp.day = t.date().day();

		rh.timeStamp.hour = t.time().hour();
		rh.timeStamp.minute = t.time().minute();
		rh.timeStamp.second = t.time().second();
		rh.timeStamp.millisecond = t.time().msec();

		FotipHeader& fh = m_reqFrame.fotip.header;

		fh.protocolVersion = 1;
		fh.subsystemKeyWord = 0;

		fh.subsystemKey.lmNumber = sr.lmNumber;
		fh.subsystemKey.subsystemCode = sr.lmSubsystemID;

		quint16 data = fh.subsystemKey.subsystemCode;	// second
		data <<= 6;
		data += fh.subsystemKey.lmNumber;			// first

		fh.subsystemKey.crc = (data << 4) % 0b10011;	// x^4+x+1

		// For IPEN only !!!! begin

		if (sr.lmNumber >= 1 && sr.lmNumber <= 4)
		{
			const quint16 subsystemKeyIPEN[4] =
			{
				0x6141,
				0x3142,
				0x0143,
				0x9144
			};

			fh.subsystemKey.wordVaue = subsystemKeyIPEN[sr.lmNumber - 1];
		}
		else
		{
			assert(false);
		}

		// For IPEN only !!!! end

		fh.operationCode = TO_INT(sr.operation);
		fh.flags.all = 0;
		fh.startAddress = sr.startAddressW;
		fh.fotipFrameSize = 1432;
		fh.romSize = sr.romSizeW * 2;					// words => bytes
		fh.romFrameSize = sr.frameSizeW * 2;			// words => bytes
		fh.dataType = sr.dataType;
		fh.uniqueId = sr.uniqueID;

		memset(fh.reserve, 0, FOTIP_HEADER_RESERVE_SIZE);
		memset(m_reqFrame.fotip.reserv, 0, FOTIP_DATA_RESERV_SIZE);
		memset(m_reqFrame.fotip.comparisonResult, 0, FOTIP_COMPARISON_RESULT_SIZE);

		if (sr.operation == Tuning::OperationCode::Write)
		{
			memcpy(m_reqFrame.fotip.data, sr.fotipData, FOTIP_TX_RX_DATA_SIZE);
		}
		else
		{
			memset(m_reqFrame.fotip.data, 0, FOTIP_TX_RX_DATA_SIZE);
		}

		if (sr.userRequest == true)
		{
			emit userRequest(m_reqFrame.fotip);
		}

		int size = sizeof(m_reqFrame);

		qint64 result = m_socket->writeDatagram(reinterpret_cast<char*>(&m_reqFrame), size, QHostAddress(sr.lmIP), sr.lmPort);

		if (result == -1)
		{
			qDebug() << "Socket write error";
		}
	}*/

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


	void TuningSourceWorkerThread::pushReply(const RupFotipFrame &reply)
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
		RupFotipFrame reply;

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


	void TuningSocketListener::pushReplyToTuningSourceWorker(const QHostAddress& tuningSourceIP, const RupFotipFrame& reply)
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


	// --------------------------------------------

	// -------------------------------------------------------------------------
	//
	//	TuningSocketRequestQueue class implementaton
	//
	// -------------------------------------------------------------------------

	TuningSocketRequestQueue::TuningSocketRequestQueue(quint32 tuningSourceIP) :
		Queue<TuningSocketRequest>(1000),
		m_tuningSourceIP(tuningSourceIP)
	{
	}


	bool TuningSocketRequestQueue::push(const TuningSocketRequest* ptr)
	{
		bool result = Queue<TuningSocketRequest>::push(ptr);

		if (result == true)
		{
			emit request(m_tuningSourceIP);
			return true;
		}

		return false;
	}


	void TuningSocketRequestQueue::stopWaiting()
	{
		m_waitingForAk = false;
		m_waitCount = 0;
	}


	void TuningSocketRequestQueue::requestIsSent()
	{
		assert(m_waitingForAk == false);

		m_waitingForAk = true;
		m_waitCount = 0;
	}


	void TuningSocketRequestQueue::incWaitCount()
	{
		assert(m_waitingForAk == true);

		m_waitCount++;
	}


	int TuningSocketRequestQueue::waitCount() const
	{
		return m_waitCount;
	}


	// -----------------------------------------------------

}

