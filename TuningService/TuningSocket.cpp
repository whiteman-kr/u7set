#include "TuningSocket.h"
#include "../lib/Types.h"
#include "TuningService.h"

namespace Tuning
{

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


	bool TuningSocketRequestQueue::isWaitingForAck() const
	{
		return m_waitingForAk;
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


	// -------------------------------------------------------------------------
	//
	//	TuningSocketWorker class implementaton
	//
	// -------------------------------------------------------------------------

	TuningSocketWorker::TuningSocketWorker(const HostAddressPort &tuningIP, const TuningSources& tuningSources) :
		m_tuningIP(tuningIP),
		m_timer100ms(this),
		m_timer1s(this)
	{
		for(const TuningSource* tuningSource : tuningSources)
		{
			if (tuningSource == nullptr)
			{
				assert(false);
			}

			// fill KEYs only!
			// queues really is allocated in createRequestQueues()
			//
			m_requestQueues.insert(tuningSource->lmAddress32(), nullptr);
		}
	}


	TuningSocketWorker::~TuningSocketWorker()
	{
	}


	void TuningSocketWorker::sendRequest(const TuningSocketRequest &socketRequest)
	{
		TuningSocketRequestQueue* queue = m_requestQueues.value(socketRequest.lmIP, nullptr);

		if (queue == nullptr)
		{
			assert(false);
			return;
		}

		queue->push(&socketRequest);
	}


	void TuningSocketWorker::clear()
	{
		for(TuningSocketRequestQueue* queue : m_requestQueues)
		{
			if (queue != nullptr)
			{
				delete queue;
			}
		}

		m_requestQueues.clear();
	}


	void TuningSocketWorker::onThreadStarted()
	{
		createRequestQueues();

		startTimers();
	}

	void TuningSocketWorker::onThreadFinished()
	{
		m_timer100ms.stop();
		m_timer1s.stop();

		closeSocket();

		clear();
	}


	void TuningSocketWorker::createRequestQueues()
	{
		QList<quint32> tuningSourcesIPs = m_requestQueues.keys();

		for(quint32 tuningSourceIP : tuningSourcesIPs)
		{
			TuningSocketRequestQueue* requestQueue = new TuningSocketRequestQueue(tuningSourceIP);

			m_requestQueues.insert(tuningSourceIP, requestQueue);

			connect(requestQueue, &TuningSocketRequestQueue::request, this, &TuningSocketWorker::onRequest);
		}
	}


	void TuningSocketWorker::startTimers()
	{
		connect(&m_timer1s, &QTimer::timeout, this, &TuningSocketWorker::onTimer1s);
		m_timer1s.setInterval(1000);
		m_timer1s.start();

		connect(&m_timer100ms, &QTimer::timeout, this, &TuningSocketWorker::onTimer100ms);
		m_timer100ms.setInterval(100);
		m_timer100ms.start();
	}


	void TuningSocketWorker::createAndBindSocket()
	{
		if (m_socket == nullptr)
		{
			m_socket = new QUdpSocket(this);

			qDebug() << "DataChannel: listening socket created";

			connect(m_socket, &QUdpSocket::readyRead, this, &TuningSocketWorker::onSocketReadyRead);
		}

		if (m_socketBound == false)
		{
			m_socketBound = m_socket->bind(m_tuningIP.address(), m_tuningIP.port());

			if (m_socketBound == true)
			{
				QString str = QString("DataChannel: socket bound on %1 - OK").arg(m_tuningIP.addressPortStr());
				qDebug() << C_STR(str);
			}
		}
	}


	void TuningSocketWorker::closeSocket()
	{
		if (m_socket != nullptr)
		{
			m_socket->close();
			delete m_socket;
			m_socket = nullptr;
		}

		m_socketBound = false;
	}


	void TuningSocketWorker::onTimer100ms()
	{
		for(TuningSocketRequestQueue* queue : m_requestQueues)
		{
			if (queue == nullptr)
			{
				assert(false);
				continue;
			}


			// TO DO !!!! MOVE THIS CONDITION DOWN !!!

			if (queue->isEmpty())
			{
				continue;
			}

			// Queue is not empty
			//

			if (queue->isWaitingForAck())
			{
				queue->incWaitCount();

				if (queue->waitCount() < MAX_WAIT_COUNT)
				{
					continue;			// continue wait
				}

				//
				// TO DO: send NoRespond to TuningSources
				//

				queue->stopWaiting();
			}

			TuningSocketRequest request;

			queue->pop(&request);

			//
			// TO DO: send request in socket
			//

			queue->requestIsSent();
		}
	}


	void TuningSocketWorker::onTimer1s()
	{
		createAndBindSocket();
	}


	void TuningSocketWorker::onRequest(quint32 tuningSourceIP)
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

/*		m_requests.pop(&sr);

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
		}*/
	}


	void TuningSocketWorker::onSocketReadyRead()
	{
		if (m_socket == nullptr)
		{
			assert(false);
			return;
		}

		QHostAddress from;

		qint64 size = m_socket->pendingDatagramSize();

		if (size > sizeof(m_ackFrame))
		{
			assert(false);
			m_socket->readDatagram(reinterpret_cast<char*>(&m_ackFrame), sizeof(m_ackFrame), &from);
			qDebug() << "TuningSocketWorker: datagram too big";
			return;
		}

		qint64 result = m_socket->readDatagram(reinterpret_cast<char*>(&m_ackFrame), sizeof(m_ackFrame), &from);

		if (result == -1)
		{
			closeSocket();
			qDebug() << "TuningSocketWorker: read socket error";
			return;
		}
/*
		SocketReply* sr = m_replies.beginPush();

		sr->lmIP = from.toIPv4Address();

		memcpy(&sr->fotipHeader, &m_ackFrame.fotip.header, sizeof(FotipHeader));
		memcpy(sr->fotipData, m_ackFrame.fotip.data, FOTIP_TX_RX_DATA_SIZE);
		memcpy(sr->fotipComparisonResult, m_ackFrame.fotip.comparisonResult, FOTIP_COMPARISON_RESULT_SIZE);

		if (m_ackFrame.fotip.header.flags.all != 0)
		{
			emit replyWithNoZeroFlags(m_ackFrame.fotip);

			qDebug() << QString("FOTIP Flags == 0x%1").arg(QString::number(m_ackFrame.fotip.header.flags.all, 16));
		}

		m_replies.completePush();*/
	}


	bool TuningSocketWorker::getReply(SocketReply* reply)
	{
		return true;
		//return m_replies.pop(reply);
	}

}
