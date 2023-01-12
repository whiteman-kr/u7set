#pragma once

#include <QObject>
#include <QtGlobal>
#include <QTimer>
#include <QHash>
#include <QMutex>
#include <QTcpServer>
#include <QSslSocket>
#include <QSslKey>
#include <cassert>

#include "../Proto/serialization.pb.h"
#include "../Proto/network.pb.h"

#include "../UtilsLib/SimpleThread.h"
#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/Crc.h"
#include "../CommonLib/HostAddressPort.h"

#include "SocketIO.h"
#include "CircularLogger.h"
#include "SoftwareInfo.h"

namespace Tcp
{
	const int TCP_MAX_DATA_SIZE = 4 * 1024 * 1024;				// 4 Mb

	// timeouts in milliseconds
	//
	const int TCP_SERVER_REPLY_TIMEOUT = 3000;
	const int TCP_CLIENT_REQUEST_TIMEOUT = 5000;

	const int TCP_PERIODIC_TIMER_INTERVAL = 500;

	const int TCP_AUTO_ACK_TIMER_INTERVAL = 500;

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

	enum class SetConnectionResult
	{
		Undefined,

		UnknownClientID,
		WrongClientHostname,

		Ok
	};


	class SocketWorker : public SimpleThreadWorker
	{
		Q_OBJECT

	public:
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

			void calcCRC();
			bool checkCRC();
		};

		#pragma pack(pop)

	public:
		SocketWorker(const SoftwareInfo& softwareInfo);
		virtual ~SocketWorker();

		bool isConnected() const;							// returns True if isSocketConnected() == True and
															// requests/reply RQID_SECURITY_LEVEL and RQID_INTRODUCE_MYSELF
															// successfully processed
		void closeConnection();

		virtual void onInitConnection();
		virtual void onConnection();
		virtual void onDisconnection();

//		int watchdogTimerTimeout() const { return m_timeout; }
//		void setWatchdogTimerTimeout(int timeout_ms) { m_timeout = timeout_ms; }
		void enableWatchdogTimer(bool enable);

		void setLogger(CircularLoggerShared logger);

		HostAddressPort localAddressPort() const;

		ConnectionState getConnectionState() const;

		SoftwareInfo localSoftwareInfo() const;
		SoftwareInfo connectedSoftwareInfo() const;

		HostAddressPort peerAddr() const;

		void setSslCertificateFileName(const QString& fileName);
		void setSslPrivateKeyFileName(const QString& fileName);

		virtual void logError(const QString& err);
		virtual void logWarning(const QString& wrn);
		virtual void logMessage(const QString& msg);

	signals:
		void socketDisconnected(const SocketWorker* socketWorker);
		void closeConnectionSignal();

	protected:
		virtual void createSocket();
		void deleteSocket();

		bool isSocketConnected() const;						// returns True if socket established TCP connection

		QString sslModeStr(QSslSocket::SslMode mode) const;

		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		virtual void onHeaderAndDataReady() {}

		qint64 socketWrite(const char* data, qint64 size);
		qint64 socketWrite(const Header& header);

		void addSentBytes(qint64 bytes);
		void addReceivedBytes(qint64 bytes);

		void addRequest();
		void addReply();

		void setStateConnected(const HostAddressPort& peerAddr);
		void setStateDisconnected();

		void startTimeoutTimer();
		void stopTimeoutTimer();

		bool loadCertificate(bool isClient);

		virtual void onConnectionEncrypted() {}

	protected slots:
		virtual void onTimeoutTimer();

	private:
		int readHeader(int bytesAvailable);
		int readData(int bytesAvailable);

		virtual void initReadStatusVariables() = 0;

	private slots:

		// QTcpSocket processing slots
		//
		virtual void stateChanged(QAbstractSocket::SocketState newState);
		virtual void connected();
		virtual void disconnected();
		virtual void readyRead();
		virtual void errorOccurred(QAbstractSocket::SocketError socketError);

		// QSslSocket processing slots
		//
		virtual void encrypted();
		virtual void sslErrors(const QList<QSslError>& errors);
		virtual void handshakeInterruptedOnError(const QSslError& error);
		virtual void modeChanged(QSslSocket::SslMode mode);
		virtual void peerVerifyError(const QSslError& error);
		virtual void preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator* authenticator);

		void onCloseConnection();

	protected:
		enum ReadState
		{
			WaitingForHeader,
			WaitingForData,
			WaitingNothing
		};

		//

		mutable QRecursiveMutex m_mutex;

		QSslSocket* m_socket = nullptr;

		int m_connNo = 0;
		E::SecurityLevel m_securityLevel = E::SecurityLevel::Basic;

		SetConnectionResult m_setConnResult = SetConnectionResult::Undefined;

		//

		mutable QMutex m_stateMutex;
		ConnectionState m_state;

		//

		QSslCertificate m_cert;			// self-signed or trusted (CA) SSL certificate
		QSslKey m_pkey;					// private key of m_cert

		QString m_certificateFileName;
		QString m_privateKeyFileName;

		std::set<QSslError::SslError> m_ignoredSslErrors;

		//

		bool m_enableTimeoutTimer = true;
		QTimer m_timeoutTimer;
		int m_timeout = TCP_CLIENT_REQUEST_TIMEOUT;			// ms

		// read-status variables
		//
		ReadState m_readState = ReadState::WaitingForHeader;
		quint32 m_readHeaderSize = 0;
		quint32 m_readDataSize = 0;

		//

		Header m_header;
		char* m_receiveDataBuffer = nullptr;

		bool m_headerAndDataReady = false;					// set to TRUE when full header and data read from socket

	private:
		std::shared_ptr<CircularLogger> m_log;
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::Server class declaration
	//
	// -------------------------------------------------------------------------------------

	class Server : public SocketWorker
	{
		Q_OBJECT

	public:
		Server(const SoftwareInfo& sotwareInfo, E::SecurityLevel securityLevel);
		virtual ~Server();

		virtual Server* getNewInstance() = 0;	// ServerDerivedClass::getNewInstance() must be implemented as:
												// { return new ServerDerivedClass(); }

		void setConnectedSocketDescriptor(qintptr connectedSocketDescriptor);

		int id() const { return m_connNo; }

		E::SecurityLevel securityLevel() const { return m_securityLevel; }

		virtual void onServerThreadStarted() {}
		virtual void onServerThreadFinished() {}

		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) = 0;

		virtual void onConnectedSoftwareInfoChanged();		// called after processing RQID_INTRODUCE_MYSELF

		void setAutoAck(bool autoAck) { m_autoAck = autoAck; }

		void sendAck();
		bool sendReply();
		bool sendReply(const QByteArray& replyData);
		bool sendReply(google::protobuf::Message& protobufMessage);
		bool sendReply(const char* replyData, quint32 replyDataSize);

		void sendClientList();

	public slots:
		void updateClientsInfo(const std::list<Tcp::ConnectionState> connectionStates);	// Update connection states of all clients from listener

	signals:
		void connectedSoftwareInfoChanged();	// Inform listener that some connection state changed

	protected:
		virtual Tcp::SetConnectionResult checkClient(const QString& clientEquipmentID, const QString& clientHostname) const;

	protected:
		std::list<Tcp::ConnectionState> m_connectionStates;

	private:
		virtual void onThreadStarted() final;
		virtual void onThreadFinished() final;

		virtual void initReadStatusVariables() final;

		virtual void createSocket() final;

		void onHeaderAndDataReady() final;

		void processSecurityLevelRequest();
		void processIntroduceMyselfRequest(const char* dataBuffer, int dataSize);

	private slots:
		void onAutoAckTimer();
		void onTimeoutTimer() override;

	private:
		enum ServerState
		{
			WainigForRequest,
			RequestProcessing,
		};

		static int m_staticConnNo;

		qintptr m_connectedSocketDescriptor = 0;

		ServerState m_serverState = ServerState::WainigForRequest;

		double m_requestProcessingPorgress = 0;

		bool m_autoAck = true;

		QTimer m_autoAckTimer;

		char* m_protobufBuffer = nullptr;

		QMutex m_statesMutex;
	};

	// -------------------------------------------------------------------------------------
	//
	// Tcp::TcpServer class declaration and implementation
	//
	// Deriving from QTcpServer required to overload incomingConnection()
	// because new connected socket will be used in another thread and not in Listener's thread
	//
	// -------------------------------------------------------------------------------------

	class TcpServer : public QTcpServer
	{
		Q_OBJECT

	public:
		TcpServer(QObject* parent) : QTcpServer(parent) {}

		virtual void incomingConnection(qintptr socketDescriptor) final
		{
			emit newConnection(socketDescriptor);
		}

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
		Listener(const HostAddressPort& listenAddressPort, Server* server, CircularLoggerShared logger);
		virtual ~Listener();

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

	private slots:
		void onPeriodicTimer();
		void onServerDisconnected(const SocketWorker *server);
		void updateClientsList();

	private:
		HostAddressPort m_listenAddressPort;
		CircularLoggerShared m_log;

		TcpServer* m_tcpServer = nullptr;

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

		virtual ~ServerThread();
	};

	// -------------------------------------------------------------------------------------
	//
	// Tcp::ClientWorker class declaration
	//
	// -------------------------------------------------------------------------------------

	class Client : public SocketWorker
	{
		Q_OBJECT

	public:
		Client(const SoftwareInfo& softwareInfo,
			   const HostAddressPort& serverAddressPort,
			   const QString& clientDescription);

		Client(const SoftwareInfo& softwareInfo,
			   const HostAddressPort& serverAddressPort1,
			   const HostAddressPort& serverAddressPort2,
			   const QString& clientDescription);

		virtual ~Client() override;

		void setServers(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2, bool reconnect);

		QString equipmentID() const;

		HostAddressPort currentServerAddressPort() const;
		HostAddressPort serverAddressPort(int serverIndex) const;

		HostAddressPort serverAddressPort1() const { return serverAddressPort(0); }
		HostAddressPort serverAddressPort2() const { return serverAddressPort(1); }

		int selectedServerIndex() { return m_selectedServerIndex; }

		bool isAutoSwitchServer() const { return m_autoSwitchServer; }
		void setAutoSwitchServer(bool autoSwitch) { m_autoSwitchServer = autoSwitch; }

		virtual void onClientThreadStarted() {}
		virtual void onClientThreadFinished() {}

		virtual void onInitConnection() final;
		virtual void onDisconnection() override;

		virtual void onTryConnectToServer(const HostAddressPort& serverAddr);

		virtual void onAck(quint32 requestID, const char* replyData, quint32 replyDataSize);

		bool isClearToSendRequest() const;

		bool sendRequest(quint32 requestID);
		bool sendRequest(quint32 requestID, const QByteArray& requestData);
		bool sendRequest(quint32 requestID, const char* requestData, quint32 requestDataSize);
		bool sendRequest(quint32 requestID, google::protobuf::Message& protobufMessage);

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) = 0;
		virtual void onReplyTimeout() { qDebug() << "Reply timeout"; }

		void enableClientAliveRequest(bool enable);

		SetConnectionResult setConnectionResult() const { return m_setConnResult; }

	protected:
		virtual void onConnectionEncrypted() override;

	protected slots:
		virtual void encrypted() override;

		virtual void sslErrors(const QList<QSslError>& errors);
		virtual void errorOccurred(QAbstractSocket::SocketError socketError);
		virtual void handshakeInterruptedOnError(const QSslError& error);
		virtual void modeChanged(QSslSocket::SslMode mode);
		virtual void peerVerifyError(const QSslError& error);
		virtual void preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator* authenticator);

	signals:
		void signal_unknownClientID(QString errMsg);
		void signal_wrongClientHostname(QString errMsg);

	private slots:
		void onTimeoutTimer() override;
		void slot_onPeriodicTimer();

	private:
		void autoSwitchServer();

		void selectFirstValidServer();
		void selectNextValidServer();

		void connectToServer();

		virtual void onThreadStarted() final;
		virtual void onThreadFinished() final;

		virtual void onHeaderAndDataReady() final;

		bool processSecurityLevelReply(const char* dataBuffer, int dataSize);
		bool processIntroduceMyselfReply(const char* dataBuffer, int dataSize);

		void sendIntroduceMyselfRequest();

		virtual void initReadStatusVariables() final;

		bool sendClientAliveRequest();

	private:
		enum ClientState
		{
			ClearToSendRequest,
			WaitingForReply,
		};


		QString m_clientDescription;

		HostAddressPort m_serversAddressPort[2];
		HostAddressPort m_selectedServer;
		int m_selectedServerIndex = 0;

		QTimer m_periodicTimer;

		int m_connectTimeout = 0;
		int m_noRequestsTimeout = 0;

		bool m_enableClientAliveRequest = true;

		bool m_enableSignalUnknownClientID = true;
		bool m_enableSignalWrongClientHostname = true;

		quint32 m_requestNumerator = 1;

		Header m_sentRequestHeader;

		bool m_autoSwitchServer = true;

		ClientState m_clientState = ClientState::ClearToSendRequest;

		char* m_protobufBuffer = nullptr;
	};
}
