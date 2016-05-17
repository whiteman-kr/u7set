#pragma once

#include <QObject>
#include <QtGlobal>
#include <QTimer>
#include <QHash>
#include <QMutex>
#include <QTcpServer>
#include <QTcpSocket>
#include <cassert>
#include "../include/SocketIO.h"
#include "../include/SimpleThread.h"
#include "../include/Utils.h"


namespace Tcp
{
	const int TCP_MAX_DATA_SIZE = 4 * 1024 * 1024;				// 4 Mb
	const int TCP_ON_CLIENT_REQUEST_REPLY_TIMEOUT = 3000;		// 3 seconds
	const int TCP_BYTES_WRITTEN_TIMEOUT = 1000;					// 1 second
	const int TCP_PERIODIC_TIMER_INTERVAL = 1000;				// 1 second
	const int TCP_AUTO_ACK_TIMER_INTERVAL = 1000;				// 1 second
	const int TCP_CONNECT_TIMEOUT = 3;							// 3 seconds


	class SocketWorker : public SimpleThreadWorker
	{
		Q_OBJECT

	protected:
		#pragma pack(push, 1)

		struct Header
		{
			enum Type
			{
				Request = 1,
				Ack = 2,
				Reply = 3,
			};

			Type type = Type::Request;

			quint32 id = 0;
			quint32 dataSize = 0;
			quint32 numerator = 0;
			double requestProcessingPorgress = 0;	// for Ack & Replay headers

			quint32 CRC32 = 0;

			void calcCRC()
			{
				this->CRC32 = ::CRC32(reinterpret_cast<const char*>(this), sizeof(Header) - sizeof(quint32));
			}

			bool checkCRC()
			{
				return ::CRC32(reinterpret_cast<const char*>(this), sizeof(Header) - sizeof(quint32)) == this->CRC32;
			}
		};

		#pragma pack(pop)


		enum ReadState
		{
			WaitingForHeader,
			WaitingForData,
			WaitingAnything
		};

		QTcpSocket* m_tcpSocket = nullptr;

		QMutex m_mutex;

		// read-status variables
		//
		ReadState m_readState = ReadState::WaitingForHeader;
		quint32 m_readedHeaderSize = 0;
		quint32 m_readedDataSize = 0;

		//

		Header m_header;
		char* m_dataBuffer = nullptr;


		bool m_headerAndDataReady = false;					// set to TRUE when full header and data readed from socket

		virtual void createSocket();
		void deleteSocket();

		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		virtual void onHeaderAndDataReady() {}

		qint64 socketWrite(const char* data, qint64 size);
		qint64 socketWrite(const Header& header);

	private:
		bool m_enableSocketRead = true;

		int readHeader(int bytesAvailable);
		int readData(int bytesAvailable);

		virtual void initReadStatusVariables() = 0;

	private slots:
		void onSocketStateChanged(QAbstractSocket::SocketState newState);
		void onSocketConnected();
		void onSocketDisconnected();
		void onSocketReadyRead();

	signals:
		void disconnected(const SocketWorker* socketWorker);

	public:
		SocketWorker();
		~SocketWorker();

		bool isConnected() const;
		bool isUnconnected() const;

		void closeConnection();

		virtual void onConnection() {}
		virtual void onDisconnection() {}
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::Server class declaration
	//
	// -------------------------------------------------------------------------------------


	class Server : public SocketWorker
	{
		Q_OBJECT

	private:
		enum ServerState
		{
			WainigForRequest,
			RequestProcessing,
		};

		static int staticId;

		int m_id = 0;

		qintptr m_connectedSocketDescriptor = 0;

		ServerState m_serverState = ServerState::WainigForRequest;

		double m_requestProcessingPorgress = 0;

		bool m_autoAck = true;

		QTimer m_autoAckTimer;

		void setConnectedSocketDescriptor(qintptr connectedSocketDescriptor);

		virtual void onThreadStarted() final;
		virtual void onThreadFinished() final;

		virtual void initReadStatusVariables() final;

		virtual void createSocket() final;

		void onHeaderAndDataReady() final;

		friend class Listener;

	private slots:
		void onAutoAckTimer();

	protected:

		virtual Server* getNewInstance() = 0;	// ServerDerivedClass::getNewInstance() must be implemented as:
												// { return new ServerDerivedClass(); }
	public:
		Server();
		~Server();

		int id() const { return m_id; }

		virtual void onServerThreadStarted() {}
		virtual void onServerThreadFinished() {}

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		void setAutoAck(bool autoAck) { m_autoAck = autoAck; }

		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) = 0;

		void sendAck();
		void sendReply();
		void sendReply(const QByteArray& replyData);
		void sendReply(const char* replyData, quint32 replyDatsSize);
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::TcpServer class declaration
	//
	// -------------------------------------------------------------------------------------

	class Listener;

	class TcpServer : public QTcpServer
	{
		Q_OBJECT

	public:
		virtual void incomingConnection(qintptr socketDescriptor) final;

	signals:
		void newConnection(qintptr socketDescriptor);
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::Listener class declaration
	//
	// -------------------------------------------------------------------------------------

	class Listener : public SimpleThreadWorker
	{
		Q_OBJECT

	private:
		HostAddressPort m_listenAddressPort;
		TcpServer* m_tcpServer = nullptr;
		QTimer m_periodicTimer;

		Server* m_serverInstance = nullptr;

		QHash<const SocketWorker*, SimpleThread*> m_runningServers;

		friend class TcpServer;

	private:
		void startListening();
		void onNewConnection(qintptr socketDescriptor);

	private slots:
		void onAcceptError();
		void onPeriodicTimer();
		void onServerDisconnected(const SocketWorker *server);

	public:
		Listener(const HostAddressPort& listenAddressPort, Server* server);
		~Listener();

		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::ServerThread class declaration
	//
	// -------------------------------------------------------------------------------------

	class ServerThread : public SimpleThread
	{
		Q_OBJECT

	public:
		ServerThread(const HostAddressPort& listenAddressPort, Server* server);
		~ServerThread();
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::ClientWorker class declaration
	//
	// -------------------------------------------------------------------------------------


	class Client : public SocketWorker
	{
		Q_OBJECT

	private:
		enum ClientState
		{
			ClearToSendRequest,
			WaitingForReply,
		};

		HostAddressPort m_serversAddressPort[2];
		HostAddressPort m_selectedServer;
		int m_selectedServerIndex = 0;

		QTimer m_periodicTimer;
		QTimer m_replyTimeoutTimer;

		quint32 m_requestNumerator = 1;

		Header m_sentRequestHeader;

		bool m_autoSwitchServer = true;

		int m_connectTimeout = 0;

		ClientState m_clientState = ClientState::ClearToSendRequest;

	private:
		void autoSwitchServer();
		void selectServer(int serverIndex, bool reconnect);

		void connectToServer();

		virtual void onThreadStarted() final;
		virtual void onThreadFinished() final;

		virtual void onHeaderAndDataReady() final;

		virtual void initReadStatusVariables() final;

		void restartReplyTimeoutTimer();
		void stopReplyTimeoutTimer();

		void processAck();

	private slots:
		void onPeriodicTimer();
		void onReplyTimeoutTimer();

	public:
		Client(const HostAddressPort &serverAddressPort);
		Client(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);

		~Client();

		void setServer(const HostAddressPort& serverAddressPort, bool reconnect);
		void setServers(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2, bool reconnect);

		void selectServer1(bool reconnect) { selectServer(0, reconnect); }
		void selectServer2(bool reconnect) { selectServer(1, reconnect); }

		bool isAutoSwitchServer() const { return m_autoSwitchServer; }
		void setAutoSwitchServer(bool autoSwitch) { m_autoSwitchServer = autoSwitch; }

		virtual void onClientThreadStarted() {}
		virtual void onClientThreadFinished() {}

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		virtual void onAck(quint32 requestID, const char* replyData, quint32 replyDataSize);
		virtual void onReplyTimeout() { qDebug() << "Reply timeout"; }

		bool isClearToSendRequest() const;

		void sendRequest(quint32 requestID);
		void sendRequest(quint32 requestID, const QByteArray& requestData);
		void sendRequest(quint32 requestID, const char* requestData, quint32 requestDataSize);

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) = 0;
	};



	// -------------------------------------------------------------------------------------
	//
	// Tcp::Thread class declaration
	//
	// -------------------------------------------------------------------------------------

	typedef SimpleThread Thread;
}
