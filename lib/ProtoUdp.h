#pragma once

#include <QObject>
#include <QtGlobal>
#include <QTimer>
#include <QThread>
#include <QUuid>
#include <QHash>
#include <QUdpSocket>
#include <QMutex>
#include <cassert>
#include "../lib/SocketIO.h"
#include "../lib/SimpleThread.h"


namespace ProtoUdp
{
	#pragma pack(push, 1)


	const int DATAGRAM_SIZE = 8192;


	struct FrameHeader
	{
		enum Type
		{
			Request = 1,
			Replay = 2,
			Ack = 3
		};

		Type type = Type::Request;
		quint32 clientID = 0;
		quint32 sessionID = 0;
		quint32 requestID = 0;
		quint32 frameNumber = 0;			// zero-based!
		quint32 totalDataSize = 0;
		quint32 errorCode = 0;

		void clear()
		{
			type = Type::Request;
			clientID = 0;				// assign by client
			sessionID = 0;				// assign by server
			requestID = 0;				// assign by client
			frameNumber = 0;			// zero-based!
			totalDataSize = 0;
			errorCode = 0;
		}
	};


	const int FRAME_DATA_SIZE = DATAGRAM_SIZE - sizeof(FrameHeader);


	const int REPLY_TIMEOUT = 50;		// 50 milliseconds
	const int RETRY_COUNT = 2;


	struct Frame
	{
		FrameHeader header;

		char data[FRAME_DATA_SIZE];
	};


	#pragma pack(pop)


	// -------------------------------------------------------------------------------------
	//
	// ProtoUdpSocket class declaration
	//
	// -------------------------------------------------------------------------------------


	class Socket : public SimpleThreadWorker
	{
		Q_OBJECT

	protected:
		HostAddressPort m_serverAddress;

		QUdpSocket m_socket;
		QTimer m_timer;
		QMutex m_sync;

		QByteArray m_requestData;
		QByteArray m_replyData;

		Frame m_requestFrame;
		Frame m_replayFrame;

		int m_replayTimeout = REPLY_TIMEOUT;

		quint32 getTotalFramesNumber(quint32 dataSize);
		quint32 getFrameDataSize(quint32 frameNumber, quint32 dataSize);

		void copyDataToFrame(Frame& protoUdpFrame, const QByteArray& data);

	public:
		Socket();
		~Socket();

	public slots:
		virtual void onThreadStarted() {}
		virtual void onThreadFinished() { deleteLater(); }
	};


	enum Error
	{
		Ok,
		NoReplayFromServer
	};


	enum ClientState
	{
		ReadyToSendRequest,
		RequestSending,
		ReplyReceiving,
		ReplyReady,
	};


	struct ClientStatus
	{
		ClientState state = ClientState::ReadyToSendRequest;
		Error error = Error::Ok;

		int requestSendingProgress = 0;				// 0 - 100%
		int replayReceivingProgress = 0;			// 0 - 100%
	};


	// -------------------------------------------------------------------------------------
	//
	// ProtoUdpClient class declaration
	//
	// -------------------------------------------------------------------------------------


	class Client : public Socket
	{
		Q_OBJECT

	private:
		HostAddressPort m_firstServerAddress;
		HostAddressPort m_secondServerAddress;
		bool m_communicateWithFirstServer = true;

		ClientStatus m_status;

		quint32 m_clientID = 0;

		int m_retryCount = 0;

		quint32 m_requestID = 0;

		void continueSendRequest();
		void sendRequestFrame();

		void calcRequestSendingProgress();
		void setRequestSendingProgress(int progress) { m_status.requestSendingProgress = progress; }

		void calcReplyReceivingProgress();
		void setReplyReceivingProgress(int progress) { m_status.replayReceivingProgress = progress; }

		ClientState state() { return m_status.state; }
		void setState(ClientState state) { m_status.state = state; }

		void setError(Error error) { m_status.error = error; }

	private slots:
		void onTimerTimeout();
		void onSocketReadyRead();

		void onRestartCommunication();

	public:
		Client(const HostAddressPort& firstServerAddress, const HostAddressPort& secondServerAddress);
		~Client();

		void setServersAddresses(const HostAddressPort& firstServerAddress, const HostAddressPort& secondServerAddress);

		void switchServer();
		void switchToFirstServer();
		void switchToSecondServer();

		bool isReadyToSendRequest() { return state() == ClientState::ReadyToSendRequest; }
		void sendRequest(quint32 requestID, QByteArray& requestData);

	public slots:
		void onStartSendRequest();

	signals:
		void startSendRequest();
		void restartCommunication();
	};


	// -------------------------------------------------------------------------------------
	//
	// ProtoUdpClientThread class declaration
	//
	// -------------------------------------------------------------------------------------


	class ClientThread : public SimpleThread
	{
		Q_OBJECT

	private:
		Client* m_client = nullptr;

	public:
		ClientThread(const HostAddressPort& serverAddress);
		ClientThread(const HostAddressPort& firstServerAddress, const HostAddressPort& secondServerAddress);

		void sendRequest(quint32 requestID, QByteArray& requestData);
	};


	// -------------------------------------------------------------------------------------
	//
	// RequestProcessor class declaration
	//
	// -------------------------------------------------------------------------------------

	class Server;

	class RequestProcessor : public QObject
	{
		Q_OBJECT

	private:
		quint32 clientID = 0;

	public:
	};


	class RequestProcessingThread : public QObject
	{
		Q_OBJECT

	private:
		Server* m_server = nullptr;
		RequestProcessor* m_requestProcessor = nullptr;

	public:
		RequestProcessingThread(Server* server, RequestProcessor* requestProcessor);
		~RequestProcessingThread();

		RequestProcessor* requestProcessor() const { return m_requestProcessor; }
	};


	// -------------------------------------------------------------------------------------
	//
	// ClientHandler class declaration
	//
	// -------------------------------------------------------------------------------------

	class ClientHandler : public QObject
	{
		Q_OBJECT

	private:
		quint32 m_clientID = 0;


	};


	// -------------------------------------------------------------------------------------
	//
	// ProtoUdpServer class declaration
	//
	// -------------------------------------------------------------------------------------


	const int MAX_REQUEST_PROCESSING_THREADS_COUNT = 50;

	class Server : public Socket
	{
		Q_OBJECT

	private:
		QTimer m_periodicTimer;

		bool m_binded = false;

		Frame m_requestFrame;

		bool bind();

		int m_maxProcessingThreadsCount = MAX_REQUEST_PROCESSING_THREADS_COUNT;

		QHash<quint32, RequestProcessingThread*> m_requestProcesingThreads;

		RequestProcessor* createRequestProcessor(quint32 requestID);

//		template<typename RequestProcessorDerivedClass>
//		void registerRequestProcessor()
//		{
//			m_requestProcessorFactory.Register<RequestProcessorDerivedClass>;
//		}

	private slots:
		void onTimerTimeout();
		void onSocketReadyRead();
		void onPeriodicTimer();

	public:
		Server(const HostAddressPort& serverAddressPort);
		~Server();

	public slots:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;
	};


	// -------------------------------------------------------------------------------------
	//
	// ProtoUdpServerThread class declaration
	//
	// -------------------------------------------------------------------------------------


	class ServerThread : public QObject
	{
		Q_OBJECT

	private:
		QThread m_thread;
		Server* m_server = nullptr;

	public:
		ServerThread(const HostAddressPort& serverAddress);
		~ServerThread();

		void run();
		void quit() { m_thread.quit(); }

		template<typename RequestProcessorDerivedClass>
		void registerRequestProcessor()
		{
//			m_server->re
		}
	};
}
