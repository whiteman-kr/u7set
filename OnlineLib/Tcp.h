#pragma once

#include <QObject>
#include <QtGlobal>
#include <QTimer>
#include <QHash>
#include <QMutex>
#include <QTcpServer>
#include <QSslSocket>
#include <QSslKey>

#include "../UtilsLib/SimpleThread.h"
#include "../UtilsLib/WUtils.h"
#include "../UtilsLib/Crc.h"
#include <CommonLib/HostAddressPort.h>

#include "CircularLogger.h"
#include "SocketIO.h"
#include "SoftwareInfo.h"
#include "TcpConnectionState.h"

namespace google 
{ 
	namespace protobuf 
	{ 
		class Message; 
	} 
}

namespace Tcp
{
	const int TCP_MAX_DATA_SIZE = 4 * 1024 * 1024;				// 4 Mb

	// timeouts in milliseconds
	//
	const int TCP_SERVER_REPLY_TIMEOUT = 3000;
	const int TCP_CLIENT_REQUEST_TIMEOUT = 5000;

	const int TCP_PERIODIC_TIMER_INTERVAL = 500;

	const int TCP_AUTO_ACK_TIMER_INTERVAL = 500;


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
		SocketWorker(const SoftwareInfo& softwareInfo, const QString& socketDescription);
		virtual ~SocketWorker();

		bool isConnected() const;					// returns True if isSocketConnected() == True and
													// requests/reply RQID_SECURITY_LEVEL and RQID_INTRODUCE_MYSELF
													// successfully processed
		void closeConnection();

		virtual void onConnection() = 0;			// called after request/reply RQID_SECURITY_LEVEL and RQID_INTRODUCE_MYSELF
													// successfully processed
		virtual void onDisconnection() = 0;

		void enableWatchdogTimer(bool enable);

		void setLogger(CircularLoggerShared logger);

		HostAddressPort localAddressPort() const;

		virtual ConnectionState getConnectionState() const;

		const SoftwareInfo& localSoftwareInfo() const;
		const SoftwareInfo& connectedSoftwareInfo() const;

		HostAddressPort peerAddr() const;

		void setSslCertificateFileName(const QString& fileName);
		void setSslPrivateKeyFileName(const QString& fileName);

		// may be overridden to use logger other than CircularLogger
		//
		virtual void logError(const QString& err) const;
		virtual void logWarning(const QString& wrn) const;
		virtual void logMessage(const QString& msg) const;

		virtual QString getLogStr(const QString& str) const;

		CircularLoggerShared log();

		QString socketDescription() const;

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

		void setSocketStateConnected(const HostAddressPort& peerAddr);
		void setSocketStateDisconnected();

		void startTimeoutTimer();
		void stopTimeoutTimer();

		bool loadCertificate(bool isClient);

		QString socketStateStr(QAbstractSocket::SocketState state) const;

		virtual void onConnectionEncrypted() {}

	protected slots:
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
		virtual void onTimeoutTimer();

	private:
		int readHeader(int bytesAvailable);
		int readData(int bytesAvailable);

		virtual void initReadStatusVariables() = 0;

	protected:
		enum ReadState
		{
			WaitingForHeader,
			WaitingForData,
			WaitingNothing
		};

		//

		SoftwareInfo m_localSoftwareInfo;
		SoftwareInfo m_connectedSoftwareInfo;

		mutable QRecursiveMutex m_mutex;

		QSslSocket* m_socket = nullptr;

		int m_connNo = 0;
		E::SecurityLevel m_securityLevel = E::SecurityLevel::Basic;
		QString m_socketDescription;

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
		mutable CircularLoggerShared m_log;
	};

	class ListenAddress
	{
	public:
		ListenAddress() = default;
		ListenAddress(const QString& eqID, const HostAddressPort& addr, E::SecurityLevel level) :
			m_equipmentID(eqID), m_hostAddr(addr), m_securityLevel(level) {}

		QString equipmentID() const { return m_equipmentID; }
		HostAddressPort hostAddr() const { return m_hostAddr; }
		E::SecurityLevel securityLevel() const { return m_securityLevel; }
		bool isValid() const { return !m_equipmentID.isEmpty(); }

	private:
		QString m_equipmentID;			// Software or RequestController EquipmentID
		HostAddressPort m_hostAddr;
		E::SecurityLevel m_securityLevel = E::SecurityLevel::Basic;
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
		Server(const SoftwareInfo& sotwareInfo,
			   const QString& serverDescription);

		virtual ~Server();

		// ServerDerivedClass::getNewInstance(const ListenAddress& listenAddr) must be implemented as:
		// {
		//		Tcp::Server* newServer = new ServerDerivedClass();
		//		newServer->setListenAddress(listenAddr);
		//		return newServer;
		// }
		virtual Server* getNewInstance(const ListenAddress& listenAddr) = 0;

		void setConnectedSocketDescriptor(qintptr connectedSocketDescriptor);
		void setListenAddress(const ListenAddress& listenAddr);

		E::SecurityLevel securityLevel() const;
		int id() const { return m_connNo; }
		QString softwareEquipmentID() const;

		virtual void onServerThreadStarted() {}
		virtual void onServerThreadFinished() {}

		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) = 0;

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		virtual void onConnectedSoftwareInfoChanged();		// called after processing RQID_INTRODUCE_MYSELF

		void setAutoAck(bool autoAck) { m_autoAck = autoAck; }

		void sendAck();
		bool sendReply();
		bool sendReply(const QByteArray& replyData);
		bool sendReply(google::protobuf::Message& protobufMessage);
		bool sendReply(const char* replyData, quint32 replyDataSize);

		void sendClientList();

		void initConnectionNo();

	public slots:
		void updateClientsInfo(const std::list<Tcp::ConnectionState> connectionStates);	// Update connection states of all clients from listener

	signals:
		void connectedSoftwareInfoChanged();	// Inform listener that some connection state changed

	protected:
		virtual Tcp::SetConnectionResult checkClient(const QString& clientEquipmentID, const QString& clientHostname) const;

	private:
		virtual void onThreadStarted() override final;
		virtual void onThreadFinished() override final;

		virtual void initReadStatusVariables() override final;

		virtual void createSocket() override final;

		void onHeaderAndDataReady() override final;

		void processSecurityLevelRequest();
		void processIntroduceMyselfRequest(const char* dataBuffer, int dataSize);

	private slots:
		void onAutoAckTimer();
		void onTimeoutTimer() override;

	protected:
		std::list<Tcp::ConnectionState> m_connectionStates;

	private:
		enum ServerState
		{
			WainigForRequest,
			RequestProcessing,
		};

		static int m_staticConnNo;

		qintptr m_connectedSocketDescriptor = 0;
		ListenAddress m_listenAddress;

		ServerState m_serverState = ServerState::WainigForRequest;

		double m_requestProcessingPorgress = 0;

		bool m_autoAck = true;

		QTimer m_autoAckTimer;

		char* m_protobufBuffer = nullptr;

		QMutex m_statesMutex;
	};

	// -------------------------------------------------------------------------------------
	//
	// ListenerSocket class declaration and implementation
	//
	// Deriving from QTcpServer required to overload incomingConnection()
	// because new connected socket will be used in another thread and not in Listener's thread
	//
	// -------------------------------------------------------------------------------------

	class ListenerSocket : public QTcpServer
	{
		Q_OBJECT

	public:
		ListenerSocket(const ListenAddress& listenAddr, QObject* parent) :
			QTcpServer(parent),
			m_listenAddr(listenAddr)
		{}

		virtual void incomingConnection(qintptr socketDescriptor) override final
		{
			emit newIncomingConnection(m_listenAddr, socketDescriptor);
		}

	signals:
		void newIncomingConnection(ListenAddress listenAddr, qintptr socketDescriptor);

	private:
		ListenAddress m_listenAddr;
	};

	// -------------------------------------------------------------------------------------
	//
	// ListenerWorker class declaration and implementation
	//
	// -------------------------------------------------------------------------------------

	class ListenerWorker : public SimpleThreadWorker
	{
		Q_OBJECT

	public:
		ListenerWorker(const std::vector<ListenAddress>& listenAddresses, Server* server, CircularLoggerShared logger);
		virtual ~ListenerWorker();

		virtual void onListenerThreadStarted() {}
		virtual void onListenerThreadFinished() {}

		virtual void onStartListening(const HostAddressPort& addr, bool startOk, const QString& errStr);

	signals:
		void connectedClientsListChanged(std::list<ConnectionState> listOfClientStates);

	private:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

		void startListening();
		void onNewConnection(ListenAddress listenAddr, qintptr socketDescriptor);

	private slots:
		void onPeriodicTimer();
		void onServerDisconnected(const SocketWorker *server);
		void updateClientsList();

	private:

	private:
		std::vector<std::pair<ListenAddress, ListenerSocket*>> m_tcpServers;

		QTimer m_periodicTimer;

		Server* m_serverInstance = nullptr;

		std::map<const SocketWorker*, SimpleThread*> m_runningServers;

		friend class ListenerSocket;
	};

	// -------------------------------------------------------------------------------------
	//
	// Tcp::ListenerThread class declaration
	//
	// -------------------------------------------------------------------------------------

	class ListenerThread : public SimpleThread
	{
		Q_OBJECT

	public:
		ListenerThread(const HostAddressPort& listenAddress,
					   E::SecurityLevel securityLevel,
					   Server* server,
					   CircularLoggerShared logger);

		ListenerThread(const ListenAddress& listenAddress,
					 Server* server,
					 CircularLoggerShared logger);

		ListenerThread(const std::vector<ListenAddress>& listenAddresses,
					 Server* server,
					 CircularLoggerShared logger);

		ListenerThread(ListenerWorker* listener);

		virtual ~ListenerThread();

	private:

	};

	// -------------------------------------------------------------------------------------
	//
	// Tcp::Client class declaration
	//
	// -------------------------------------------------------------------------------------

	class Client : public SocketWorker
	{
		Q_OBJECT

	public:
		Client(const SoftwareInfo& softwareInfo,
			   const HostAddressPort& serverAddressPort,
			   const QString& clientDescription,
			   const QString& serverEquipmentID = QString());

		Client(const SoftwareInfo& softwareInfo,
			   const HostAddressPort& serverAddressPort1,
			   const HostAddressPort& serverAddressPort2,
			   const QString& clientDescription,
			   const QString& serverEquipmentID = QString());

		virtual ~Client() override;

		void setServers(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2, bool reconnect);

		QString equipmentID() const;
		QString connectToServerID() const;

		HostAddressPort currentServerAddressPort() const;
		HostAddressPort serverAddressPort(int serverIndex) const;
		HostAddressPort serverAddressPort1() const { return serverAddressPort(0); }
		HostAddressPort serverAddressPort2() const { return serverAddressPort(1); }

		int selectedServerIndex() { return m_selectedServerIndex; }

		bool isAutoSwitchServer() const { return m_autoSwitchServer; }
		void setAutoSwitchServer(bool autoSwitch) { m_autoSwitchServer = autoSwitch; }

		virtual void onClientThreadStarted() {}
		virtual void onClientThreadFinished() {}

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		virtual void onTryConnectToServer(const HostAddressPort& serverAddr);

		virtual void onAck(quint32 requestID, const char* replyData, quint32 replyDataSize);

		virtual ConnectionState getConnectionState() const override;

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
		virtual void connected() override;

	signals:
		void signal_unknownClientID(QString errMsg);
		void signal_wrongClientHostname(QString errMsg);
		void signal_wrongServerID(QString errMsg);

	private slots:
		void onTimeoutTimer() override;
		void slot_onPeriodicTimer();

	private:
		void autoSwitchServer();

		void selectFirstValidServer();
		void selectNextValidServer();

		void connectToServer();

		virtual void onThreadStarted() override final;
		virtual void onThreadFinished() override final;

		virtual void onHeaderAndDataReady() override final;

		bool processSecurityLevelReply(const char* dataBuffer, int dataSize);
		SetConnectionResult processIntroduceMyselfReply(const char* dataBuffer, int dataSize);

		void sendIntroduceMyselfRequest();

		virtual void initReadStatusVariables() override final;

		bool sendClientAliveRequest();

	private:
		enum ClientState
		{
			ClearToSendRequest,
			WaitingForReply,
		};

		HostAddressPort m_serversAddressPort[2];
		HostAddressPort m_selectedServer;
		int m_selectedServerIndex = 0;

		QString m_serverEquipmentID;

		QTimer m_periodicTimer;

		int m_connectTimeout = 0;
		int m_noRequestsTimeout = 0;

		bool m_enableClientAliveRequest = true;

		bool m_enableSignalUnknownClientID = true;
		bool m_enableSignalWrongClientHostname = true;
		bool m_enableSignalWrongServerID = true;

		quint32 m_requestNumerator = 1;

		Header m_sentRequestHeader;

		bool m_autoSwitchServer = true;

		ClientState m_clientState = ClientState::ClearToSendRequest;

		char* m_protobufBuffer = nullptr;
	};
}
