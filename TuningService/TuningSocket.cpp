#include "TuningSocket.h"
#include "../include/Types.h"

namespace Tuning
{
	// -------------------------------------------------------------------------
	//
	//	TuningSocket class implementaton
	//
	// -------------------------------------------------------------------------

	TuningSocket::TuningSocket(const HostAddressPort& tuningIP) :
		SimpleThread(new TuningSocketWorker(tuningIP))
	{
	}


	// -------------------------------------------------------------------------
	//
	//	TuningSocketWorker class implementaton
	//
	// -------------------------------------------------------------------------

	TuningSocketWorker::TuningSocketWorker(const HostAddressPort &tuningIP) :
		m_tuningIP(tuningIP),
		m_requests(50),
		m_timer(this)
	{
	}


	void TuningSocketWorker::clear()
	{
	}


	void TuningSocketWorker::onThreadStarted()
	{
		connect(&m_timer, &QTimer::timeout, this, &TuningSocketWorker::onTimer);
		connect(&m_requests, &Queue<SocketRequest>::queueNotEmpty, this, &TuningSocketWorker::onSocketRequest);

		m_timer.setInterval(1000);
		m_timer.start();

	}

	void TuningSocketWorker::onThreadFinished()
	{
		m_timer.stop();
		closeSocket();
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


	void TuningSocketWorker::onTimer()
	{
		createAndBindSocket();
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
				qDebug() << str;
			}
		}
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
			qDebug() << "DataChannel: datagram too big";
			return;
		}

		qint64 result = m_socket->readDatagram(reinterpret_cast<char*>(&m_ackFrame), sizeof(m_ackFrame), &from);

		if (result == -1)
		{
			closeSocket();
			qDebug() << "DataChannel: read socket error";
		}
	}


	void TuningSocketWorker::onSocketRequest()
	{
		SocketRequest sr;

		m_requests.pop(&sr);

		if (m_socketBound == false)
		{
			return;
		}

		int sv = sizeof(RupFotipFrame);

		RupFrameHeader& rh = m_reqFrame.rupHeader;

		rh.frameSize = ENTIRE_UDP_SIZE;
		rh.protocolVersion = 4;
		rh.flagsWord = 0;
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

		FotipHeader& fh = m_reqFrame.fotipHeader;

		fh.protocolVersion = 1;
		fh.subsystemKeyWord = 0;

		fh.subsystemKey.channelNumber = 1; //sr.channel;
		fh.subsystemKey.subsystemCode = 5;

		quint16 data = fh.subsystemKey.subsystemCode;	// second
		data <<= 6;
		data += fh.subsystemKey.channelNumber;			// first
		fh.subsystemKey.crc = (data << 4) % 0b10011;	// x^4+x+1

		fh.operationCode = TO_INT(sr.operation);
		fh.flagsWord = 0;
		fh.startAddress = sr.startAddressW;
		fh.fotipFrameSize = 1432;
		fh.romSize = sr.romSizeW * 2;					// words => bytes
		fh.romFrameSize = sr.frameSizeW * 2;			// words => bytes
		fh.dataType = sr.dataType;
		fh.uniqueId = 0;

		memset(fh.reserve, 0, FOTIP_HEADER_RESERVE_SIZE);
		memset(m_reqFrame.reserv, 0, sizeof(m_reqFrame.reserv));



		if (sr.operation == Tuning::OperationCode::Write)
		{
			memcpy(m_reqFrame.fotipData, sr.frameData, fh.romFrameSize);
		}
		else
		{
			memset(m_reqFrame.fotipData, 0, sizeof(m_reqFrame.fotipData));
		}

		int size = sizeof(m_reqFrame);

		qint64 result = m_socket->writeDatagram(reinterpret_cast<char*>(&m_reqFrame), size, QHostAddress(sr.lmIP), sr.lmPort);

		if (result == -1)
		{
			qDebug() << "Socket write error";
		}
	}

	void TuningSocketWorker::sendRequest(const SocketRequest& socketRequest)
	{
		m_requests.push(&socketRequest);
	}



	/*void TuningSocketWorker::readTunigData()
	{

	}*/


	// -------------------------------------------------------------------------
	//
	//	TuningSocketWorker::Request class implementaton
	//
	// -------------------------------------------------------------------------


	/*void TuningSocketWorker::Request::initToRead(const TuningSettings& settings, quint64 tuningID, int frameNo)
	{
		Q_UNUSED(settings);
		Q_UNUSED(frameNo);
		header.version = 1;
		header.tuningID = tuningID;
//		header.subsystemKey = subsystemKey;
//		header.operationCode = TO_INT(OperationCode::Read);
		header.flags = 0;
//		header.startAddressW = 0;			//
		header.requestSizeB;			// UDP frame size = 1432 bytes
		header.romSizeB;				// = ROM_SIZE_B
		header.romFrameSizeB;			// = ROM_FRAME_SIZE_B
		header.dataType;				// DataType enum values
	}*/


}
