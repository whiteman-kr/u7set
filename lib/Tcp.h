#pragma once

#include <QObject>
#include <QtGlobal>
#include <QTimer>
#include <QHash>
#include <QMutex>
#include <QTcpServer>
#include <QTcpSocket>
#include <cassert>

#include "../Proto/serialization.pb.h"
#include "../Proto/network.pb.h"

#include "../lib/SocketIO.h"
#include "../lib/SimpleThread.h"
#include "../lib/WUtils.h"
#include "../lib/CircularLogger.h"


namespace Tcp
{
	const int TCP_MAX_DATA_SIZE = 4 * 1024 * 1024;				// 4 Mb
	const int TCP_ON_CLIENT_REQUEST_REPLY_TIMEOUT = 3000;		// 3 seconds
	const int TCP_BYTES_WRITTEN_TIMEOUT = 1000;					// 1 second
	const int TCP_PERIODIC_TIMER_INTERVAL = 1000;				// 1 second
	const int TCP_AUTO_ACK_TIMER_INTERVAL = 1000;				// 1 second
	const int TCP_CONNECT_TIMEOUT = 3;							// 3 seconds

	class SoftwareInfo
	{
	public:
		static const int UNDEFINED_BUILD_NO;

		SoftwareInfo();
		SoftwareInfo(const SoftwareInfo& si);

		void init(E::SoftwareType softwareType,
				  const QString& equipmentID,
				  int majorVersion,
				  int minorVersion,
				  int buildNo);

		void serializeTo(Network::TcpSoftwareInfo* info);
		void serializeFrom(const Network::TcpSoftwareInfo& info);

		E::SoftwareType softwareType() const { return m_softwareType; }
		QString equipmentID() const { return m_equipmentID; }
		int majorVersion() const { return m_majorVersion; }
		int minorVersion() const { return m_minorVersion; }
		int commitNo() const { return m_commitNo; }
		QString userName() const { return m_userName; }
		int buildNo() const { return m_buildNo; }

	private:
		E::SoftwareType m_softwareType = E::SoftwareType::Unknown;
		QString m_equipmentID;
		int m_majorVersion = 0;
		int m_minorVersion = 0;
		int m_commitNo = 0;
		QString m_userName;
		int m_buildNo = UNDEFINED_BUILD_NO;
	};

	struct ConnectionState
	{
		bool isConnected = false;

		// nex data is valid if isConnected == true
		//
		HostAddressPort peerAddr;
		qint64 startTime = 0;					// milliseconds since epoch

		qint64 sentBytes = 0;
		qint64 receivedBytes = 0;

		qint64 requestCount = 0;
		qint64 replyCount = 0;

		SoftwareInfo connectedSoftwareInfo;
		SoftwareInfo localSoftwareInfo;

		bool isActual = false;

		void dump();
	};

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
			WaitingNothing
		};

		QTcpSocket* m_tcpSocket = nullptr;

		ConnectionState m_state;

		mutable QMutex m_stateMutex;
		mutable QMutex m_mutex;

		QTimer m_watchdogTimer;
		int m_watchdogTimerTimeout = 5000;			// ms
		bool m_watchdogTimerEnable = true;

		// read-status variables
		//
		ReadState m_readState = ReadState::WaitingForHeader;
		quint32 m_readHeaderSize = 0;
		quint32 m_readDataSize = 0;

		//

		bool m_bytesWritten = true;

		Header m_header;
		char* m_receiveDataBuffer = nullptr;

		bool m_headerAndDataReady = false;					// set to TRUE when full header and data read from socket

		virtual void createSocket();
		void deleteSocket();

		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		virtual void onHeaderAndDataReady() {}

		qint64 socketWrite(const char* data, qint64 size);
		qint64 socketWrite(const Header& header);

		void resetStaticstics();

		void addSentBytes(int bytes);
		void addReceivedBytes(int bytes);

		void addRequest();
		void addReply();

		void setStateConnected(const HostAddressPort& peerAddr);
		void setStateDisconnected();

	private:
		int readHeader(int bytesAvailable);
		int readData(int bytesAvailable);

		virtual void initReadStatusVariables() = 0;

	private slots:
		void onSocketStateChanged(QAbstractSocket::SocketState newState);
		void onSocketConnected();
		void onSocketDisconnected();
		void onSocketReadyRead();
		void onSocketBytesWritten();

	protected slots:
		virtual void onWatchdogTimerTimeout();

	signals:
		void disconnected(const SocketWorker* socketWorker);

	public:
		SocketWorker(const SoftwareInfo& softwareInfo);

		virtual ~SocketWorker();

		bool isConnected() const;
		bool isUnconnected() const;

		void closeConnection();

		virtual void onInitConnection();
		virtual void onConnection();
		virtual void onDisconnection();

		int watchdogTimerTimeout() const { return m_watchdogTimerTimeout; }
		void setWatchdogTimerTimeout(int timeout_ms) { m_watchdogTimerTimeout = timeout_ms; }
		void enableWatchdogTimer(bool enable);

		void restartWatchdogTimer();

		ConnectionState getConnectionState() const;

		SoftwareInfo localSoftwareInfo() const;

		HostAddressPort peerAddr() const;
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

		char* m_protobufBuffer = nullptr;

		virtual void onThreadStarted() final;
		virtual void onThreadFinished() final;

		virtual void initReadStatusVariables() final;

		virtual void createSocket() final;

		void onHeaderAndDataReady() final;

	private slots:
		void onAutoAckTimer();

	public:
		Server(const SoftwareInfo& sotwareInfo);
		virtual ~Server();

		virtual Server* getNewInstance() = 0;	// ServerDerivedClass::getNewInstance() must be implemented as:
												// { return new ServerDerivedClass(); }

		void setConnectedSocketDescriptor(qintptr connectedSocketDescriptor);

		SoftwareInfo localSoftwareInfo();

		int id() const { return m_id; }

		virtual void onServerThreadStarted() {}
		virtual void onServerThreadFinished() {}

		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) = 0;

		void setAutoAck(bool autoAck) { m_autoAck = autoAck; }

		void sendAck();
		bool sendReply();
		bool sendReply(const QByteArray& replyData);
		bool sendReply(google::protobuf::Message& protobufMessage);
		bool sendReply(const char* replyData, quint32 replyDataSize);
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

	public:
		Listener(const HostAddressPort& listenAddressPort, Server* server, std::shared_ptr<CircularLogger> logger);
		~Listener();

		virtual void onListenerThreadStarted() {}
		virtual void onListenerThreadFinished() {}

		virtual void onNewConnectionAccepted(const HostAddressPort& peerAddr, int connectionNo);
		virtual void onStartListening(const HostAddressPort& addr, bool startOk, const QString& errStr);

	signals:
		void connectedClientsListChanged(std::list<ConnectionState> listOfClientStates);

	private:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		void startListening();
		void onNewConnection(qintptr socketDescriptor);

		void updateClientsList();

	private slots:
		void onPeriodicTimer();
		void onServerDisconnected(const SocketWorker *server);

	private:
		HostAddressPort m_listenAddressPort;
		TcpServer* m_tcpServer = nullptr;

		std::shared_ptr<CircularLogger> m_logger;

		QTimer m_periodicTimer;

		Server* m_serverInstance = nullptr;

		QHash<const SocketWorker*, SimpleThread*> m_runningServers;

		friend class TcpServer;
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
		ServerThread(const HostAddressPort& listenAddressPort,
					 Server* server,
					 std::shared_ptr<CircularLogger> logger);
		ServerThread(Listener* listener);

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

		char* m_protobufBuffer = nullptr;

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

	protected slots:
		virtual void onWatchdogTimerTimeout() override;

	public:
		Client(const HostAddressPort& serverAddressPort,
			   const SoftwareInfo& softwareInfo);

		Client(const HostAddressPort& serverAddressPort1,
			   const HostAddressPort& serverAddressPort2,
			   const SoftwareInfo& softwareInfo);

		virtual ~Client();

		void setServer(const HostAddressPort& serverAddressPort, bool reconnect);
		void setServers(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2, bool reconnect);

		void selectServer1(bool reconnect) { selectServer(0, reconnect); }
		void selectServer2(bool reconnect) { selectServer(1, reconnect); }

		QString equipmentID() const;

		HostAddressPort currentServerAddressPort();
		HostAddressPort serverAddressPort(int serverIndex);
		int selectedServerIndex() { return m_selectedServerIndex; }

		bool isAutoSwitchServer() const { return m_autoSwitchServer; }
		void setAutoSwitchServer(bool autoSwitch) { m_autoSwitchServer = autoSwitch; }

		virtual void onClientThreadStarted() {}
		virtual void onClientThreadFinished() {}

		virtual void onInitConnection() final;
		virtual void onDisconnection() override;

		virtual void onTryConnectToServer(const HostAddressPort& serverAddr);

		virtual void onAck(quint32 requestID, const char* replyData, quint32 replyDataSize);
		virtual void onReplyTimeout() { qDebug() << "Reply timeout"; }

		bool isClearToSendRequest() const;

		bool sendRequest(quint32 requestID);
		bool sendRequest(quint32 requestID, const QByteArray& requestData);
		bool sendRequest(quint32 requestID, const char* requestData, quint32 requestDataSize);
		bool sendRequest(quint32 requestID, google::protobuf::Message& protobufMessage);

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) = 0;
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::Thread class declaration
	//
	// -------------------------------------------------------------------------------------

	typedef SimpleThread Thread;
}
