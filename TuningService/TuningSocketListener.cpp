#include "../lib/Types.h"
#include "TuningService.h"
#include "TuningSourceWorker.h"
#include "TuningSocketListener.h"

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

	TuningSocketListener::TuningSocketListener(const HostAddressPort &listenIP, const TuningSources& tuningSources) :
		m_listenIP(listenIP),
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

		TuningSourceWorker* sourceWorker = m_ipTuningSourceWorkerMap.value(sourceIP, nullptr);

		if (sourceWorker == nullptr)
		{
			m_errUnknownTuningSource++;
			return;
		}

		sourceWorker->pushReply(reply);
	}
}
