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


namespace Tcp
{
	const int TCP_PACKET_MIN_DATA_SIZE = 128 * 1024;					// 128 Kb
	const int TCP_PACKET_MAX_DATA_SIZE = TCP_PACKET_MIN_DATA_SIZE * 16;	// 2 Mb
	const int TCP_ON_CLIENT_REQUEST_REPLY_TIMEOUT = 2000;				// 2 seconds


	class SocketWorker : public SimpleThreadWorker
	{
		Q_OBJECT

	private:
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
			WainigForHeader,
			WainigForData,
		};

		enum State
		{
			// server states
			//
			WainigForRequest,
			RequestProcessing,

			// client states
			//
			ClearToSendRequest,
			WaitingForReply,
		};

		bool m_isServer = true;

		qintptr m_connectedSocketDescriptor = 0;	// used only by server socket
		QTcpSocket* m_tcpSocket = nullptr;

		State m_state = State::WainigForRequest;
		ReadState m_readState = ReadState::WainigForHeader;

		quint32 m_numerator = 1;					// used only by client socket

		Header m_header;
		char* m_dataBuffer = nullptr;
		quint32 m_dataBufferSize = 0;				// current size of allocated m_dataBuffer

		quint32 m_readedHeaderSize = 0;
		quint32 m_readedDataSize = 0;

		bool m_requestReady = false;				// set to TRUE when full request (header and data), ack or reply readed from socket

		QTimer m_replyTimeoutTimer;					// used only by client socket

	private:
		void reallocateDataBuffer(int newDataBufferSize);

		int readHeader(int bytesAvailable);
		int readData(int bytesAvailable);

		void onRequestReady();

	protected:
		virtual void onThreadStarted() override;
		virtual void onThreadFinished() override;

	private slots:
		void onSocketStateChanged(QAbstractSocket::SocketState newState);
		void onSocketConnected();
		void onSocketDisconnected();
		void onSocketReadyRead();
		void onReplyTimeoutTimer();

	signals:
		void disconnected(const SocketWorker* server);

	public:
		SocketWorker(bool isServer);
		~SocketWorker();

		void setConnectedSocketDescriptor(qintptr connectedSocketDescriptor);

		bool isServer() const { return m_isServer; }
		bool isClient() const { return !m_isServer; }

		bool isConnected() const;
		bool isUnconnected() const;
		bool isClearToSendRequest() const;

		void closeConnection();

		void sendRequest(quint32 requestID, const QByteArray& requestData);
		void sendRequest(quint32 requestID, const char* requestData, quint32 requestDatsSize);

		void sendAck();
		void sendReply(const char* replyData, quint32 replyDatsSize);

		virtual void omSocketThreadStarted() {}
		virtual void omSocketThreadFinished() {}
		virtual void onConnection() {}
		virtual void onDisconnection() {}
		virtual void onReplyTimeout() {}

		virtual void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) {}
		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) {}
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
		static int staticId;

		int m_id = 0;

		virtual void onThreadStarted() final;
		virtual void onThreadFinished() final;

		friend class Listener;

	protected:

		virtual Server* getNewInstance() = 0;	// ServerDerivedClass::getNewInstance() must be implemented as:
												// { return new ServerDerivedClass(); }

	public:
		Server();
		~Server();

		int id() const { return m_id; }

		virtual void onServerThreadStarted() {}
		virtual void onServerThreadFinished() {}
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::TcpServer class declaration
	//
	// -------------------------------------------------------------------------------------

	class Listener;

	class TcpServer : public QTcpServer
	{
	public:
		TcpServer(Listener* parent);
		virtual void incomingConnection(qintptr socketDescriptor) override;
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
		TcpServer m_tcpServer;
		QTimer m_periodicTimer;

		Server* m_serverInstance = nullptr;

		QHash<const SocketWorker*, SimpleThread*> m_runningServices;

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
		HostAddressPort m_serversAddressPort[2];
		HostAddressPort m_selectedServer;

		QTimer m_periodicTimer;

	private:
		void connectToServer();

		virtual void onThreadStarted() final;
		virtual void onThreadFinished() final;

	private slots:
		void onPeriodicTimer();

	public:
		Client();
		~Client();

		virtual void onSocketThreadStarted() {}
		virtual void onSocketThreadFinished() {}

		virtual void onConnection() {}
		virtual void onDisonnection() {}

		virtual void onRequestTimeout() {}


		void setServer(const HostAddressPort& serverAddressPort);
		void setServers(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);

		void selectServer1() { m_selectedServer = m_serversAddressPort[0]; }
		void selectServer2() { m_selectedServer = m_serversAddressPort[1]; }
	};



	// -------------------------------------------------------------------------------------
	//
	// Tcp::ClientThread class declaration
	//
	// -------------------------------------------------------------------------------------


	template <typename ClientDerivedClass>
	class ClientThread : public SimpleThread
	{
	public:
		ClientThread(const HostAddressPort& serverAddressPort);
		ClientThread(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2);

		~ClientThread();
	};


	// -------------------------------------------------------------------------------------
	//
	// Tcp::ClientThread class implementation
	//
	// -------------------------------------------------------------------------------------

	template <typename ClientDerivedClass>
	ClientThread<ClientDerivedClass>::ClientThread(const HostAddressPort &serverAddressPort) :
		SimpleThread(new ClientDerivedClass)
	{
		Client* client = dynamic_cast<Client*>(m_worker);

		if (client != nullptr)
		{
			client->setServer(serverAddressPort);
		}
		else
		{
			assert(false);
		}
	}


	template <typename ClientDerivedClass>
	ClientThread<ClientDerivedClass>::ClientThread(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
		SimpleThread(new ClientDerivedClass)
	{
		Client* client = dynamic_cast<Client*>(m_worker);

		if (client != nullptr)
		{
			client->setServers(serverAddressPort1, serverAddressPort2);
		}
		else
		{
			assert(false);
		}
	}


	template <typename ClientDerivedClass>
	ClientThread<ClientDerivedClass>::~ClientThread()
	{
	}

}
