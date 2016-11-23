#pragma once

#include "TuningSource.h"

namespace Tuning
{

	// ----------------------------------------------------------------------------------
	//
	// TuningSourceWorker class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSourceWorker : public SimpleThreadWorker
	{
		Q_OBJECT

	public:
		TuningSourceWorker(const TuningSource& source);

		quint32 sourceIP() const;

		void pushReply(const RupFotipFrame& reply);


	signals:
		void replyReady();

	private:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		bool processWaitReply();
		bool processCommandQueue();
		bool processIdle();

		void processReply();

	private slots:
		void onTimer();
		void onReplyReady();

	private:
		const TuningSource& m_source;

		HostAddressPort m_sourceIP;

		bool m_waitReply = false;

		int m_waitReplyCounter = 0;

		const int MAX_WAIT_REPLY_COUNTER = 3;

		QMutex m_waitReplyMutex;

		RupFotipFrame m_request;
		RupFotipFrame m_reply;

		// statisticts
		//
		qint64 m_requestCount = 0;
		qint64 m_replyCount = 0;

		qint64 m_errUntimelyReplay = 0;
	};


	// ----------------------------------------------------------------------------------
	//
	// TuningSourceWorkerThread class declaration
	//
	// ----------------------------------------------------------------------------------


	class TuningSourceWorkerThread : public SimpleThread
	{
	public:
		TuningSourceWorkerThread(const TuningSource& source);
		~TuningSourceWorkerThread();

		quint32 sourceIP();

	private:
		TuningSourceWorker* m_sourceWorker = nullptr;
	};


	typedef QHash<quint32, TuningSourceWorkerThread*> TuningSourceWorkerThreadMap;


	// ----------------------------------------------------------------------------------
	//
	// TuningSocketListener class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSocketListener : public SimpleThreadWorker
	{
		Q_OBJECT

	public:
		TuningSocketListener(const HostAddressPort& listenIP, const TuningSourceWorkerThreadMap& sourceWorkerMap);
		~TuningSocketListener();

	signals:

	private:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		void createSocket();
		void closeSocket();

		void startTimer();

	private slots:
		void onTimer();

		void onSocketReadyRead();

		void pushReplyToTuningSourceWorker(const QHostAddress& tuningSourceIP, const RupFotipFrame& reply);

	private:
		HostAddressPort m_listenIP;
		const TuningSourceWorkerThreadMap& m_sourceWorkerMap;

		QTimer m_timer;

		QUdpSocket* m_socket = nullptr;

		QHash<quint32, TuningSourceWorker*> m_ipTuningSourceWorkerMap;

		// statistics
		//
		qint64 m_errReplySize = 0;
		qint64 m_errReadSocket = 0;
		qint64 m_errUnknownTuningSource = 0;
	};


	// ----------------------------------------------------------------------------------
	//
	// TuningSocketListenerThread class declaration
	//
	// ----------------------------------------------------------------------------------

	class TuningSocketListenerThread : public SimpleThread
	{
	public:
		TuningSocketListenerThread(const HostAddressPort& listenIP, const TuningSourceWorkerThreadMap& sourceWorkerMap);
		~TuningSocketListenerThread();

	private:
		TuningSocketListener* m_socketListener = nullptr;
	};
}
