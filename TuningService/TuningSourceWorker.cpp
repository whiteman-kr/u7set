#include "../lib/WUtils.h"
#include "TuningSourceWorker.h"


namespace Tuning
{

	TuningSourceWorker::TuningSourceWorker(const TuningSource& source) :
		m_source(source)
	{
	}


	quint32 TuningSourceWorker::sourceIP() const
	{
		return m_source.lmAddress32();
	}


	void TuningSourceWorker::pushReply(const RupFotipFrame& reply)
	{
		AUTO_LOCK(m_waitReplyMutex)

		if (m_waitReply == false)
		{
			m_errUntimelyReplay++;
			return;
		}

		memcpy(&m_reply, &reply, sizeof(RupFotipFrame));

		emit replyReady();
	}


	void TuningSourceWorker::onThreadStarted()
	{
		connect(this, &TuningSourceWorker::replyReady, this, &TuningSourceWorker::onReplyReady);
	}


	void TuningSourceWorker::onThreadFinished()
	{
	}


	bool TuningSourceWorker::processWaitReply()
	{
		AUTO_LOCK(m_waitReplyMutex)

		if (m_waitReply == true)
		{
			m_waitReplyCounter++;

			if (m_waitReplyCounter < MAX_WAIT_REPLY_COUNTER)
			{
				return true;
			}

			// fix replay timeout
			//
		}

		m_waitReply = false;

		return false;			// switch to next processing
	}


	bool TuningSourceWorker::processCommandQueue()
	{
		AUTO_LOCK(m_waitReplyMutex)

		if (m_waitReply == true)
		{
			return true;		// while wating reply has not another processing
		}

		// get commands from queue and send FOTIP request
		//


		return false;			// switch to next processing
	}


	bool TuningSourceWorker::processIdle()
	{
		AUTO_LOCK(m_waitReplyMutex)

		if (m_waitReply == true)
		{
			return true;		// while wating reply has not another processing
		}

		return false;
	}


	void TuningSourceWorker::processReply()
	{

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
		m_replyCount++;

		processReply();

		AUTO_LOCK(m_waitReplyMutex)

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

}
