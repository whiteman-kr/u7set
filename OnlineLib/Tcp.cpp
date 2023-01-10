#ifndef ONLINE_LIB_DOMAIN
#error Don't include this file in the project! Link OnlineLib instead.
#endif

#include "Tcp.h"
#include "../lib/ConstStrings.h"
#include "../Proto/network.pb.h"
#include <cstdlib>

namespace Tcp
{
	void ConnectionState::dump()
	{
		if (isConnected == false)
		{
			qDebug() << "\nTcp::ConnectionState - is not connected\n";
		}
		else
		{
			qDebug() << "\nTcp::ConnectionState - is connected";
			qDebug() << qPrintable(QString("Peer: %1").arg(peerAddr.addressPortStr()));
			qDebug() << qPrintable(QString("Start time: %1").arg(QDateTime::fromMSecsSinceEpoch(startTime).toString()));
			qDebug() << qPrintable(QString("Sent bytes: %1").arg(sentBytes));
			qDebug() << qPrintable(QString("Received bytes: %1").arg(receivedBytes));
			qDebug() << qPrintable(QString("Request count: %1").arg(requestCount));
			qDebug() << qPrintable(QString("Reply count: %1\n").arg(replyCount));
		}
	}

	void SocketWorker::Header::calcCRC()
	{
		this->CRC32 = ::CRC32(reinterpret_cast<const char*>(this), sizeof(Header) - sizeof(quint32));
	}

	bool SocketWorker::Header::checkCRC()
	{
		return ::CRC32(reinterpret_cast<const char*>(this), sizeof(Header) - sizeof(quint32)) == this->CRC32;
	}

	// -------------------------------------------------------------------------------------
	//
	// Tcp::SocketWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	SocketWorker::SocketWorker(const SoftwareInfo& softwareInfo) :
		m_timeoutTimer(this)
	{
		m_state.localSoftwareInfo = softwareInfo;

		m_receiveDataBuffer = new char[TCP_MAX_DATA_SIZE];
	}

	SocketWorker::~SocketWorker()
	{
		delete [] m_receiveDataBuffer;

		Q_ASSERT(m_socket == nullptr);
	}

	bool SocketWorker::isConnected() const
	{
		AUTO_LOCK(m_mutex)

		if (m_socket == nullptr)
		{
			return false;
		}

		return m_socket->state() == QAbstractSocket::ConnectedState;
	}

	void SocketWorker::closeConnection()
	{
		emit closeConnectionSignal();
	}

	void SocketWorker::onInitConnection()
	{
		if (m_socket == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		qDebug() << C_STR(QString(tr("Socket connected with %1 (descriptor = %2)")).
						  arg(peerAddr().addressStr()).
						  arg(m_socket->socketDescriptor()));
	}

	void SocketWorker::onConnection()
	{
		qDebug() << C_STR(QString(tr("Socket connected (descriptor = %1)")).
						  arg(m_socket->socketDescriptor()));
	}

	void SocketWorker::onDisconnection()
	{
		qDebug() << C_STR(QString(tr("Socket disconnected (descriptor = %1)")).
						  arg(m_socket->socketDescriptor()));
	}

	void SocketWorker::enableWatchdogTimer(bool enable)
	{
		AUTO_LOCK(m_mutex);

		m_enableTimeoutTimer = enable;

		if (enable == false)
		{
			m_timeoutTimer.stop();
		}
	}

	void SocketWorker::setLogger(CircularLoggerShared logger)
	{
		m_log = logger;
	}

	HostAddressPort SocketWorker::localAddressPort() const
	{
		AUTO_LOCK(m_mutex);

		if (m_socket == nullptr)
		{
			return HostAddressPort();
		}

		QHostAddress locAddr = m_socket->localAddress();
		quint16 locPort = m_socket->localPort();

		return HostAddressPort(locAddr, locPort);
	}

	ConnectionState SocketWorker::getConnectionState() const
	{
		m_stateMutex.lock();

		ConnectionState state = m_state;

		m_stateMutex.unlock();

		return state;
	}

	SoftwareInfo SocketWorker::localSoftwareInfo() const
	{
		return getConnectionState().localSoftwareInfo;
	}

	SoftwareInfo SocketWorker::connectedSoftwareInfo() const
	{
		return getConnectionState().connectedSoftwareInfo;
	}

	HostAddressPort SocketWorker::peerAddr() const
	{
		m_stateMutex.lock();

		HostAddressPort peerAddr = m_state.peerAddr;

		m_stateMutex.unlock();

		return peerAddr;
	}

	QString SocketWorker::sslModeStr(QSslSocket::SslMode mode) const
	{
		switch(mode)
		{
		case QSslSocket::SslMode::UnencryptedMode:
			return QString("UnencryptedMode");

		case QSslSocket::SslMode::SslServerMode:
			return QString("SslServerMode");

		case QSslSocket::SslMode::SslClientMode:
			return QString("SslClientMode");

		default:
			Q_ASSERT(false);
		}

		return QString();
	}

	void SocketWorker::logError(const QString& err)
	{
		if (m_log == nullptr)
		{
			qDebug() << err.toStdString().c_str();
		}
		else
		{
			DEBUG_LOG_ERR(m_log, err);
		}
	}

	void SocketWorker::logWarning(const QString& wrn)
	{
		if (m_log == nullptr)
		{
			qDebug() << wrn.toStdString().c_str();
		}
		else
		{
			DEBUG_LOG_WRN(m_log, wrn);
		}
	}

	void SocketWorker::logMessage(const QString& msg)
	{
		if (m_log == nullptr)
		{
			qDebug() << msg.toStdString().c_str();
		}
		else
		{
			DEBUG_LOG_MSG(m_log, msg);
		}
	}

	void SocketWorker::createSocket()
	{
		AUTO_LOCK(m_mutex)

		deleteSocket();

		m_socket = new QSslSocket;

		m_socket->setSocketOption(QAbstractSocket::LowDelayOption, QVariant(1));

		connect(m_socket, &QSslSocket::stateChanged, this, &SocketWorker::onSocketStateChanged);
		connect(m_socket, &QSslSocket::connected, this, &SocketWorker::onSocketConnected);
		connect(m_socket, &QSslSocket::disconnected, this, &SocketWorker::onSocketDisconnected);
		connect(m_socket, &QSslSocket::readyRead, this, &SocketWorker::onSocketReadyRead);
		connect(m_socket, &QSslSocket::bytesWritten, this, &SocketWorker::onSocketBytesWritten);
	}

	void SocketWorker::deleteSocket()
	{
		if (m_socket != nullptr)
		{
			m_socket->close();
			delete m_socket;
			m_socket = nullptr;
		}
	}

	void SocketWorker::onThreadStarted()
	{
		createSocket();

		connect(&m_timeoutTimer, &QTimer::timeout, this, &SocketWorker::onTimeoutTimer);
		connect(this, &SocketWorker::closeConnectionSignal, this, &SocketWorker::onCloseConnection);
	}

	void SocketWorker::onThreadFinished()
	{
		deleteSocket();
	}

	qint64 SocketWorker::socketWrite(const char* data, qint64 size)
	{
		if (m_socket == nullptr)
		{
			Q_ASSERT(false);
			return -1;
		}

		qint64 written = m_socket->write(data, size);

		if (written == -1)
		{
			return -1;
		}

		m_bytesWritten = false;

		addSentBytes(size);

		return written;
	}

	qint64 SocketWorker::socketWrite(const Header& header)
	{
		return socketWrite(reinterpret_cast<const char*>(&header), sizeof(header));
	}

	void SocketWorker::addSentBytes(qint64 bytes)
	{
		AUTO_LOCK(m_stateMutex);

		m_state.sentBytes += bytes;
	}

	void SocketWorker::addReceivedBytes(qint64 bytes)
	{
		AUTO_LOCK(m_stateMutex);

		m_state.receivedBytes += bytes;
	}

	void SocketWorker::addRequest()
	{
		AUTO_LOCK(m_stateMutex);

		m_state.requestCount++;
	}

	void SocketWorker::addReply()
	{
		AUTO_LOCK(m_stateMutex);

		m_state.replyCount++;
	}

	void SocketWorker::setStateConnected(const HostAddressPort& peerAddr)
	{
		AUTO_LOCK(m_stateMutex);

		m_state.isConnected = true;
		m_state.peerAddr = peerAddr;
		m_state.startTime = QDateTime::currentMSecsSinceEpoch();
		m_state.sentBytes = 0;
		m_state.receivedBytes = 0;
		m_state.requestCount = 0;
		m_state.replyCount = 0;
	}

	void SocketWorker::setStateDisconnected()
	{
		AUTO_LOCK(m_stateMutex);

		m_state.isConnected = false;
		m_state.peerAddr.clear();
		m_state.startTime = 0;
		m_state.sentBytes = 0;
		m_state.receivedBytes = 0;
		m_state.requestCount = 0;
		m_state.replyCount = 0;
	}

	void SocketWorker::startTimeoutTimer()
	{
		if (m_enableTimeoutTimer == true)
		{
			m_timeoutTimer.setSingleShot(true);
			m_timeoutTimer.start(m_timeout);
		}
	}

	void SocketWorker::stopTimeoutTimer()
	{
		m_timeoutTimer.stop();
	}

	bool SocketWorker::loadCertificate(bool isClient)
	{
		m_cert.clear();
		m_pkey.clear();

		QString appPath = QCoreApplication::applicationDirPath();

		QString certFilePath;
		QString pkeyFilePath;

		switch(m_securityLevel)
		{
		case E::SecurityLevel::Basic:
											// No certificates required
			return true;

		case E::SecurityLevel::Encoded:

			if (isClient == true)
			{
				// In this mode Client not require certificate
				//
				return true;
			}

			// Server use self-signed certificate
			//
			certFilePath = appPath + File::CRYPTO_SS_SERVER_CERTIFICATE;
			pkeyFilePath = appPath + File::CRYPTO_SS_SERVER_PRIVATE_KEY;

			break;

		case E::SecurityLevel::SSL:

			// In this mode both Server and Client require trusted (CA) certificates
			//
			if (isClient == true)
			{
				certFilePath = appPath + File::CRYPTO_CA_CLIENT_CERTIFICATE;
				pkeyFilePath = appPath + File::CRYPTO_CA_CLIENT_PRIVATE_KEY;
			}
			else
			{
				certFilePath = appPath + File::CRYPTO_CA_SERVER_CERTIFICATE;
				pkeyFilePath = appPath + File::CRYPTO_CA_SERVER_PRIVATE_KEY;
			}

			break;

		default:
			Q_ASSERT(false);
			return false;
		}

		bool result = true;

		QDir dir;

		if (dir.exists(certFilePath) == false)
		{
			logError(QString("Certificate file %1 not found!").arg(certFilePath));
			result = false;
		}

		if (dir.exists(pkeyFilePath) == false)
		{
			logError(QString("Private key file %1 not found!").arg(pkeyFilePath));
			result = false;
		}

		RETURN_IF_FALSE(result);

		// Private Key loading
		//
		QFile pkeyFile(pkeyFilePath);

		if (pkeyFile.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text) == false)
		{
			logError(QString("Can't open private key file %1 !").arg(pkeyFilePath));
			return false;
		}

		m_pkey = QSslKey(pkeyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);

		if (m_pkey.isNull() == true)
		{
			logError(QString("Private key %1 loading error or key file is corrupted!").arg(pkeyFilePath));
			return false;
		}
		else
		{
			logMessage(QString("Server private key loaded."));
		}

		// Certificate loading
		//
		QFile certFile(certFilePath);

		if (certFile.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text) == false)
		{
			logError(QString("Can't open certificate file %1 !").arg(certFilePath));
			return false;
		}

		m_cert = QSslCertificate(certFile.readAll(), QSsl::Pem);

		if (m_cert.isNull() == true)
		{
			logError(QString("Certificate %1 loading error or certificate file is corrupted!").arg(certFilePath));
			return false;
		}

		if (m_cert.isSelfSigned() == true)
		{
			logMessage(QString("Self-signed certificate loaded. Issuer - %1.").arg(m_cert.issuerDisplayName()));
		}
		else
		{
			logMessage(QString("CA certificate loaded. Issuer - %1.").arg(m_cert.issuerDisplayName()));
		}

		m_socket->setLocalCertificate(m_cert);
		m_socket->setPrivateKey(m_pkey);

		return true;
	}

	void SocketWorker::onTimeoutTimer()
	{
		qDebug() << "SocketWorker::onTimeoutTimer()";
	}


	int SocketWorker::readHeader(int bytesAvailable)
	{
		if (m_readState != ReadState::WaitingForHeader)
		{
			Q_ASSERT(false);
			return 0;
		}

		int bytesToRead = sizeof(SocketWorker::Header) - m_readHeaderSize;

		if (bytesToRead > bytesAvailable)
		{
			bytesToRead = bytesAvailable;
		}

		qint64 bytesRead = m_socket->read(reinterpret_cast<char*>(&m_header) + m_readHeaderSize, bytesToRead);

		m_readHeaderSize += static_cast<quint32>(bytesRead);

		Q_ASSERT(m_readHeaderSize <= sizeof(SocketWorker::Header));

		if (m_readHeaderSize < sizeof(SocketWorker::Header))
		{
			return static_cast<int>(bytesRead);
		}

		// Full requestHeader is read
		//
		if (m_header.checkCRC() == false)
		{
			Q_ASSERT(false);

			closeConnection();

			qDebug() << "Request header CRC error!";

			return 0;
		}

		if (m_header.dataSize == 0)
		{
			m_headerAndDataReady = true;

			m_readState = ReadState::WaitingNothing;

			return static_cast<int>(bytesRead);
		}

		if (m_header.dataSize > TCP_MAX_DATA_SIZE)
		{
			Q_ASSERT(false);

			closeConnection();

			qDebug() << "Request" << m_header.id << "dataSize too big - " << m_header.dataSize;

			return 0;
		}

		m_readState = ReadState::WaitingForData;

		return static_cast<int>(bytesRead);
	}

	int SocketWorker::readData(int bytesAvailable)
	{
		if (m_readState != ReadState::WaitingForData)
		{
			Q_ASSERT(false);
			return 0;
		}

		int bytesToRead = m_header.dataSize - m_readDataSize;

		if (bytesToRead > bytesAvailable)
		{
			bytesToRead = bytesAvailable;
		}

		if (m_readDataSize + bytesToRead > TCP_MAX_DATA_SIZE)
		{
			Q_ASSERT(false);

			closeConnection();

			qDebug() << "Out of buffer m_requestData";

			return 0;
		}

		qint64 bytesRead = m_socket->read(m_receiveDataBuffer + m_readDataSize, bytesToRead);

		m_readDataSize += static_cast<quint32>(bytesRead);

		Q_ASSERT(m_readDataSize <= m_header.dataSize);

		if (m_readDataSize == m_header.dataSize)
		{
			m_headerAndDataReady = true;

			m_readState = ReadState::WaitingNothing;
		}

		return static_cast<int>(bytesRead);
	}


	void SocketWorker::onSocketStateChanged(QAbstractSocket::SocketState newState)
	{
		Q_UNUSED(newState);
		return;			// its Ok!

//		QString stateStr;

//		switch(newState)
//		{
//		case QAbstractSocket::UnconnectedState:
//			stateStr = "Socket state: UnconnectedState";
//			break;

//		case QAbstractSocket::HostLookupState:
//			stateStr = "Socket state: HostLookupState";
//			break;

//		case QAbstractSocket::ConnectingState:
//			stateStr = "Socket state: ConnectingState";
//			break;

//		case QAbstractSocket::ConnectedState:
//			stateStr = "Socket state: ConnectedState";
//			break;

//		case QAbstractSocket::BoundState:
//			stateStr = "Socket state: BoundState";
//			break;

//		case QAbstractSocket::ClosingState:
//			stateStr = "Socket state: ClosingState";
//			break;

//		default:
//			Q_ASSERT(false);
//		}

//		qDebug() << qPrintable(stateStr);
	}

	void SocketWorker::onSocketConnected()
	{
		initReadStatusVariables();

		setStateConnected(HostAddressPort(m_socket->peerAddress(), m_socket->peerPort()));

		onInitConnection();
	}

	void SocketWorker::onSocketDisconnected()
	{
		onDisconnection();

		setStateDisconnected();

		emit disconnected(this);
	}

	void SocketWorker::onSocketReadyRead()
	{
		if (m_socket == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		qint64 bytesAvailable = m_socket->bytesAvailable();

		addReceivedBytes(bytesAvailable);

		int bytesRead = 0;

		while(bytesAvailable > 0)
		{
			switch(m_readState)
			{
			case ReadState::WaitingNothing:
				Q_ASSERT(false);
				return;

			case ReadState::WaitingForHeader:
				bytesRead = readHeader(static_cast<int>(bytesAvailable));
				break;

			case ReadState::WaitingForData:
				bytesRead = readData(static_cast<int>(bytesAvailable));
				break;

			default:
				Q_ASSERT(false);
			}

			bytesAvailable -= bytesRead;

			if (m_headerAndDataReady)
			{
				// prepare to read next request
				//
				m_headerAndDataReady = false;
				m_readHeaderSize = 0;
				m_readDataSize = 0;

				onHeaderAndDataReady();
			}
		}
	}

	void SocketWorker::onSocketBytesWritten()
	{
		m_bytesWritten = true;
	}

	void SocketWorker::onCloseConnection()
	{
		if (m_socket == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		m_socket->close();
	}

	// -------------------------------------------------------------------------------------
	//
	// Tcp::Server class implementation
	//
	// -------------------------------------------------------------------------------------

	int Server::staticId = 0;

	Server::Server(const SoftwareInfo& sotwareInfo, E::SecurityLevel securityLevel) :
		SocketWorker(sotwareInfo),
		m_autoAckTimer(this)
	{
		m_securityLevel = securityLevel;

		m_timeout = TCP_CLIENT_REQUEST_TIMEOUT;

		m_id = staticId;
		staticId++;

		initReadStatusVariables();

		m_autoAckTimer.setSingleShot(false);
	}

	Server::~Server()
	{
		if (m_protobufBuffer != nullptr)
		{
			delete [] m_protobufBuffer;
		}
	}

	void Server::setConnectedSocketDescriptor(qintptr connectedSocketDescriptor)
	{
		m_connectedSocketDescriptor = connectedSocketDescriptor;
	}

	void Server::onConnectedSoftwareInfoChanged()
	{
		// called after processing RQID_INTRODUCE_MYSELF
	}

	void Server::sendAck()
	{
		if (m_socket == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		if (m_serverState != Server::ServerState::RequestProcessing)
		{
			return;
		}

		SocketWorker::Header header;

		header.type = SocketWorker::Header::Type::Ack;
		header.id = m_header.id;
		header.numerator = m_header.numerator;
		header.dataSize = 0;
		header.requestProcessingPorgress = m_requestProcessingPorgress;
		header.calcCRC();

		qint64 written = socketWrite(header);

		if (written == -1)
		{
			qDebug() << qPrintable(QString("Socket write error: %1").arg(m_socket->errorString()));
			return;
		}

		if (written < static_cast<qint64>(sizeof(header)))
		{
			Q_ASSERT(false);
			return;
		}
	}

	bool Server::sendReply()
	{
		return sendReply(nullptr, 0);
	}

	bool Server::sendReply(const QByteArray& replyData)
	{
		return sendReply(replyData.constData(), static_cast<quint32>(replyData.size()));
	}

	bool Server::sendReply(google::protobuf::Message& protobufMessage)
	{
		int messageSize = static_cast<int>(protobufMessage.ByteSizeLong());

		if (messageSize > TCP_MAX_DATA_SIZE)
		{
			Q_ASSERT(false);
			return false;
		}

		if (m_protobufBuffer == nullptr)
		{
			m_protobufBuffer = new char [TCP_MAX_DATA_SIZE];

			Q_ASSERT(m_protobufBuffer != nullptr);
		}

		protobufMessage.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(m_protobufBuffer));

		return sendReply(m_protobufBuffer, messageSize);
	}

	bool Server::sendReply(const char* replyData, quint32 replyDataSize)
	{
		m_autoAckTimer.stop();

		if (m_socket == nullptr)
		{
			Q_ASSERT(false);
			return false;
		}

		if (m_serverState != ServerState::RequestProcessing)
		{
			Q_ASSERT(false);
			return false;
		}

		addReply();

		SocketWorker::Header header;

		header.type = SocketWorker::Header::Type::Reply;
		header.id = m_header.id;
		header.numerator = m_header.numerator;
		header.dataSize = replyDataSize;
		header.requestProcessingPorgress = 100;
		header.calcCRC();

		qint64 written = socketWrite(header);

		if (written == -1)
		{
			qDebug() << qPrintable(QString("Socket write error: %1").arg(m_socket->errorString()));
			return false;
		}

		if (written < static_cast<qint64>(sizeof(header)))
		{
			Q_ASSERT(false);
			return false;
		}

		if (replyDataSize > 0)
		{
			written = socketWrite(replyData, replyDataSize);

			if (written == -1)
			{
				qDebug() << qPrintable(QString("Socket write error: %1").arg(m_socket->errorString()));
				return false;
			}

			if (written < replyDataSize)
			{
				Q_ASSERT(false);
				return false;
			}
		}

		m_socket->flush();

		initReadStatusVariables();

		return true;
	}

	void Server::sendClientList()
	{
		Network::ServiceClients message;

		m_statesMutex.lock();

		for(const Tcp::ConnectionState& state : m_connectionStates)
		{
			const SoftwareInfo& si = state.connectedSoftwareInfo;

			if (E::contains<E::SoftwareType>(TO_INT(si.softwareType())) == false)
			{
				continue;
			}

			Network::ServiceClientInfo* clientInfo = message.add_clients();

			Network::SoftwareInfo* newSoftwareInfo = new Network::SoftwareInfo();

			si.serializeTo(newSoftwareInfo);

			clientInfo->set_allocated_softwareinfo(newSoftwareInfo);

			clientInfo->set_ip(state.peerAddr.address32());

			clientInfo->set_uptime(QDateTime::currentMSecsSinceEpoch() - state.startTime);
			clientInfo->set_isactual(state.isActual);
			clientInfo->set_replyquantity(state.replyCount);
		}

		m_statesMutex.unlock();

		sendReply(message);
	}

	void Server::updateClientsInfo(const std::list<Tcp::ConnectionState> connectionStates)
	{
		m_statesMutex.lock();

		m_connectionStates = connectionStates;

		m_statesMutex.unlock();
	}

	Tcp::SetConnectionResult Server::checkClient(const QString& clientEquipmentID, const QString& clientHostname) const
	{
		Q_UNUSED(clientEquipmentID);
		Q_UNUSED(clientHostname);

		return Tcp::SetConnectionResult::Ok;			// real checking will be implemented in derived classes (if required)
	}

	void Server::encrypted()
	{
		logMessage(QString("Connection %1 encrypted").arg(m_id));
	}

	void Server::sslErrors(const QList<QSslError>& errors)
	{
		logError(QString("Ssl errors occured on connection #%1:").
							arg(m_id));

		for(const QSslError& err : errors)
		{
			logError(QString("(%1) %2").
						  arg(static_cast<int>(err.error())).arg(err.errorString()));

/*			switch(err.error())
			{
			case QSslError::SelfSignedCertificate:
				m_socket.ignoreSslErrors();
			}*/
		}
	}

	void Server::errorOccurred(QAbstractSocket::SocketError socketError)
	{
		logError(QString("Connection %1 errorOccured: (%2) %3").
								arg(m_id).arg(static_cast<int>(socketError)).arg(m_socket->errorString()));
	}

	void Server::handshakeInterruptedOnError(const QSslError& error)
	{
		logError(QString("Connection %1 handshake interrupted on error: (%2) %3").
						arg(m_id).arg(static_cast<int>(error.error())).arg(error.errorString()));
	}

	void Server::modeChanged(QSslSocket::SslMode mode)
	{
		logMessage(QString("Connection %1 mode changed to %2").
						arg(m_id).arg(sslModeStr(mode)));
	}

	void Server::peerVerifyError(const QSslError &error)
	{
		logError(QString("Connection %1 peer verify error: (%2) %3").
					arg(m_id).arg(static_cast<int>(error.error())).arg(error.errorString()));
	}

	void Server::preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator* authenticator)
	{
		Q_UNUSED(authenticator);

		logWarning(QString("Connection %1 preSharedKeyAuthenticationRequired").arg(m_id));
	}

	void Server::onThreadStarted()
	{
		connect(&m_autoAckTimer, &QTimer::timeout, this, &Server::onAutoAckTimer);

		SocketWorker::onThreadStarted();

		connect(m_socket, &QSslSocket::encrypted, this, &Server::encrypted);
		connect(m_socket, &QSslSocket::sslErrors, this, &Server::sslErrors);
		connect(m_socket, &QSslSocket::errorOccurred, this, &Server::errorOccurred);
		connect(m_socket, &QSslSocket::handshakeInterruptedOnError, this, &Server::handshakeInterruptedOnError);
		connect(m_socket, &QSslSocket::modeChanged, this, &Server::modeChanged);
		connect(m_socket, &QSslSocket::peerVerifyError, this, &Server::peerVerifyError);
		connect(m_socket, &QSslSocket::preSharedKeyAuthenticationRequired, this, &Server::preSharedKeyAuthenticationRequired);

		onServerThreadStarted();

		onConnection();

		startTimeoutTimer();
	}

	void Server::onThreadFinished()
	{
		onServerThreadFinished();

		SocketWorker::onThreadFinished();
	}

	void Server::initReadStatusVariables()
	{
		m_serverState = ServerState::WainigForRequest;
		m_readState = ReadState::WaitingForHeader;
		m_readHeaderSize = 0;
		m_readDataSize = 0;
	}

	void Server::createSocket()
	{
		Q_ASSERT(m_connectedSocketDescriptor != 0);

		SocketWorker::createSocket();

		m_socket->setSocketDescriptor(m_connectedSocketDescriptor);

		// added 20_01_2017 by WhiteMan
		//
		setStateConnected(HostAddressPort(m_socket->peerAddress(), m_socket->peerPort()));
	}

	void Server::onHeaderAndDataReady()
	{
		Q_ASSERT(m_serverState == ServerState::WainigForRequest);

		if (m_header.type != Header::Request)
		{
			Q_ASSERT(false);
			return;
		}

		stopTimeoutTimer();

		m_serverState = ServerState::RequestProcessing;

		if (m_autoAck == true)
		{
			m_autoAckTimer.start(TCP_AUTO_ACK_TIMER_INTERVAL);
		}

		m_requestProcessingPorgress = 0;

		addRequest();

		switch(m_header.id)
		{
		case RQID_SECURITY_LEVEL:
			processSecurityLevelRequest();
			break;

		case RQID_INTRODUCE_MYSELF:
			processIntroduceMyselfRequest(m_receiveDataBuffer, m_header.dataSize);
			break;

		case TCP_CLIENT_ALIVE:
			// Wow! Client still alive!
			// nothing to do, only restart timeout timer
			//
			//qDebug() << "receive TCP_CLIENT_ALIVE";

			// reply on TCP_CLIENT_ALIVE request is not required
			//
			initReadStatusVariables();

			break;

		default:
			processRequest(m_header.id, m_receiveDataBuffer, m_header.dataSize);
		}

		startTimeoutTimer();
	}

	void Server::processSecurityLevelRequest()
	{
		qint32 securityLevelInt = static_cast<qint32>(m_securityLevel);

		sendReply(reinterpret_cast<const char*>(&securityLevelInt), sizeof (securityLevelInt));

		switch(m_securityLevel)
		{
		case E::SecurityLevel::Basic:
			break;

		case E::SecurityLevel::Encoded:
		case E::SecurityLevel::SSL:

			if (loadCertificate(false) == false)
			{
				closeConnection();
				return;
			}

			m_socket->setPeerVerifyMode(m_securityLevel == E::SecurityLevel::Encoded ?
											QSslSocket::VerifyNone :
											QSslSocket::VerifyPeer);

			m_socket->startServerEncryption();

			break;

		default:
			Q_ASSERT(false);
		}
	}

	void Server::processIntroduceMyselfRequest(const char* dataBuffer, int dataSize)
	{
		Network::IntroduceMyselfRequest request;

		bool result = request.ParseFromArray(dataBuffer, dataSize);

		if (result == false)
		{
			Q_ASSERT(false);
			return;
		}

		m_stateMutex.lock();

		m_state.connectedSoftwareInfo.serializeFrom(request.clientsoftwareinfo());

		Network::IntroduceMyselfReply reply;

		m_state.localSoftwareInfo.serializeTo(reply.mutable_serversoftwareinfo());

		Tcp::SetConnectionResult err = checkClient(m_state.connectedSoftwareInfo.equipmentID(),
												  m_state.connectedSoftwareInfo.hostname());
		m_stateMutex.unlock();

		reply.set_setconnectionerror(static_cast<::google::protobuf::int32>(err));

		switch(err)
		{
		case Tcp::SetConnectionResult::Ok:
			break;

		case Tcp::SetConnectionResult::UnknownClientID:
			reply.set_errormsg((QString("Unknown client equipmentID: %1").
								arg(m_state.connectedSoftwareInfo.equipmentID())).toStdString());
			break;

		case Tcp::SetConnectionResult::WrongClientHostname:
			reply.set_errormsg((QString("Client %1 running on computer with wrong hostname").
								arg(m_state.connectedSoftwareInfo.equipmentID())).toStdString());
			break;

		default:
			Q_ASSERT(false);
		}

		sendReply(reply);

		onConnectedSoftwareInfoChanged();

		emit connectedSoftwareInfoChanged();
	}


	void Server::onAutoAckTimer()
	{
		if (m_autoAck == false || m_serverState != ServerState::RequestProcessing)
		{
			m_autoAckTimer.stop();

			return;
		}

		sendAck();
	}

	void Server::onTimeoutTimer()
	{
		qDebug() << "Tcp::Server::onTimeoutTimer()";

		closeConnection();
	}

	// -------------------------------------------------------------------------------------
	//
	// Tcp::Listener class implementation
	//
	// -------------------------------------------------------------------------------------

	Listener::Listener(const HostAddressPort& listenAddressPort, Server* server, CircularLoggerShared logger) :
		m_listenAddressPort(listenAddressPort),
		m_log(logger),
		m_periodicTimer(this),
		m_serverInstance(server)
	{
		Q_ASSERT(m_serverInstance != nullptr);

		qRegisterMetaType<std::list<ConnectionState>>("std::list<ConnectionState>");

		m_serverInstance->setParent(this);
	}

	Listener::~Listener()
	{
		// close all conection threads
		//
		for(SimpleThread* connectionThread : m_runningServers)
		{
			connectionThread->quit();
			delete connectionThread;
		}

		m_runningServers.clear();

		delete m_serverInstance;
	}

	void Listener::onNewConnectionAccepted(const HostAddressPort& peerAddr, int connectionNo)
	{
		Q_UNUSED(peerAddr)
		Q_UNUSED(connectionNo)
	}

	void Listener::onStartListening(const HostAddressPort& addr, bool startOk, const QString& errStr)
	{
		if (startOk == true)
		{
			DEBUG_LOG_MSG(m_log, QString("Start listening %1 OK").arg(addr.addressPortStr()));
		}
		else
		{
			DEBUG_LOG_ERR(m_log, QString("Error on start listening %1: %2").arg(addr.addressPortStr()).arg(errStr));
		}
	}

	void Listener::onThreadStarted()
	{
		m_periodicTimer.setInterval(TCP_PERIODIC_TIMER_INTERVAL);

		connect(&m_periodicTimer, &QTimer::timeout, this, &Listener::onPeriodicTimer);
		connect(&m_periodicTimer, &QTimer::timeout, this, &Listener::updateClientsList);

		m_periodicTimer.start();

		startListening();

		onListenerThreadStarted();
	}

	void Listener::onThreadFinished()
	{
		onListenerThreadFinished();

		if (m_tcpServer != nullptr)
		{
			m_tcpServer->close();
			delete m_tcpServer;
		}
	}

	void Listener::startListening()
	{
		if (m_tcpServer == nullptr)
		{
			m_tcpServer = new TcpServer(this);

			connect(m_tcpServer, &TcpServer::newConnection, this, &Listener::onNewConnection);
		}

		if (m_tcpServer->listen(m_listenAddressPort.address(), m_listenAddressPort.port()) == true)
		{
			onStartListening(m_listenAddressPort, true, "");
		}
		else
		{
			onStartListening(m_listenAddressPort, false, m_tcpServer->errorString());
		}
	}

	void Listener::onNewConnection(qintptr socketDescriptor)
	{
		// accept new connection
		//
		Server* newServerInstance = m_serverInstance->getNewInstance();

		newServerInstance->setLogger(m_log);

		connect(this, &Listener::connectedClientsListChanged, newServerInstance, &Server::updateClientsInfo);

		connect(newServerInstance, &Server::disconnected, this, &Listener::onServerDisconnected);
		connect(newServerInstance, &Server::connectedSoftwareInfoChanged, this, &Listener::updateClientsList);

		newServerInstance->setConnectedSocketDescriptor(socketDescriptor);

		onNewConnectionAccepted(newServerInstance->peerAddr(), newServerInstance->id());

		SimpleThread* newThread = new SimpleThread(newServerInstance);

		m_runningServers.insert(newServerInstance, newThread);

		newThread->start();

		updateClientsList();
	}

	void Listener::onPeriodicTimer()
	{
		if (!m_tcpServer->isListening())
		{
			startListening();
		}
	}

	void Listener::onServerDisconnected(const SocketWorker* server)
	{
		SimpleThread* thread = m_runningServers.value(server, nullptr);

		if (thread == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		m_runningServers.remove(server);

		thread->quit();
		delete thread;

		updateClientsList();
	}

	void Listener::updateClientsList()
	{
		std::list<ConnectionState> clientsInfo;

		QList<const SocketWorker*>&& servers = m_runningServers.keys();

		for (const SocketWorker* server : servers)
		{
			clientsInfo.push_back(server->getConnectionState());
		}

		emit connectedClientsListChanged(clientsInfo);
	}

	// -------------------------------------------------------------------------------------
	//
	// Tcp::ServerThread class implementation
	//
	// -------------------------------------------------------------------------------------

	ServerThread::ServerThread(const HostAddressPort &listenAddressPort,
							   Server* server,
							   std::shared_ptr<CircularLogger> logger) :
		SimpleThread(new Listener(listenAddressPort, server, logger))
	{
	}

	ServerThread::ServerThread(Listener* listener) :
		SimpleThread(listener)
	{
	}

	ServerThread::~ServerThread()
	{
	}

	// -------------------------------------------------------------------------------------
	//
	// Tcp::ClientWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	const std::set<QSslError::SslError> Client::m_ignoredErrors = {
																	QSslError::SslError::SelfSignedCertificate,
																	QSslError::SslError::HostNameMismatch
																  };

	Client::Client(const SoftwareInfo& softwareInfo,
				   const HostAddressPort &serverAddressPort,
				   const QString& clientDescription) :
		SocketWorker(softwareInfo),
		m_clientDescription(clientDescription),
		m_periodicTimer(this)
	{
		m_timeout = TCP_SERVER_REPLY_TIMEOUT;

		setServers(serverAddressPort, serverAddressPort,false);
		initReadStatusVariables();
	}


	Client::Client(const SoftwareInfo& softwareInfo,
				   const HostAddressPort& serverAddressPort1,
				   const HostAddressPort& serverAddressPort2, const QString &clientDescription) :
		SocketWorker(softwareInfo),
		m_clientDescription(clientDescription),
		m_periodicTimer(this)
	{
		m_timeout = TCP_SERVER_REPLY_TIMEOUT;

		setServers(serverAddressPort1, serverAddressPort2, false);
		initReadStatusVariables();
	}

	Client::~Client()
	{
		if (m_protobufBuffer != nullptr)
		{
			delete [] m_protobufBuffer;
		}
	}

	void Client::setServers(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2, bool reconnect)
	{
		AUTO_LOCK(m_mutex)

		m_serversAddressPort[0] = serverAddressPort1;
		m_serversAddressPort[1] = serverAddressPort2;

		selectFirstValidServer();

		if (reconnect == true)
		{
			closeConnection();
		}
	}

	QString Client::equipmentID() const
	{
		return localSoftwareInfo().equipmentID();
	}

	HostAddressPort Client::currentServerAddressPort() const
	{
		return m_selectedServer;
	}

	HostAddressPort Client::serverAddressPort(int serverIndex) const
	{
		if (serverIndex < 0 || serverIndex > 1)
		{
			Q_ASSERT(false);
			return HostAddressPort();
		}

		return m_serversAddressPort[serverIndex];
	}

	void Client::onInitConnection()
	{
		if (objectName().isEmpty() == true)
		{
			qDebug() << qPrintable(QString("Socket connected to server %1").arg(m_selectedServer.addressPortStr()));
		}
		else
		{
			qDebug() << qPrintable(QString("%1: Socket connected to server %2")
										.arg(objectName())
										.arg(m_selectedServer.addressPortStr()));
		}

		sendRequest(RQID_SECURITY_LEVEL);
	}

	void Client::onDisconnection()
	{
		if (objectName().isEmpty() == true)
		{
			qDebug() << qPrintable(QString("Socket disconnected from server %1").arg(m_selectedServer.addressPortStr()));
		}
		else
		{
			qDebug() << qPrintable(QString("%1: Socket disconnected from server %2")
										.arg(objectName())
										.arg(m_selectedServer.addressPortStr()));
		}
	}

	void Client::onTryConnectToServer(const HostAddressPort& serverAddr)
	{
		if (objectName().isEmpty() == true)
		{
			if (serverAddr.isSet() == true)
			{
				qDebug() << qPrintable(QString("Try connect to server %1").arg(serverAddr.addressPortStr()));
			}
			else
			{
				qDebug() << qPrintable(QString("IP address of server is NOT SET! Connection is impossible!"));
			}
		}
		else
		{
			if (serverAddr.isSet() == true)
			{
				qDebug() << qPrintable(QString("%1: Try connect to server %2")
											.arg(objectName())
											.arg(serverAddr.addressPortStr()));
			}
			else
			{
				qDebug() << qPrintable(QString("%1: IP address of server is NOT SET! Connection is impossible!")
											.arg(objectName()));
			}
		}
	}

	void Client::onAck(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		Q_UNUSED(requestID);
		Q_UNUSED(replyData);
		Q_UNUSED(replyDataSize);
	}

	bool Client::isClearToSendRequest() const
	{
		return isConnected() && m_clientState == ClientState::ClearToSendRequest;
	}

	bool Client::sendRequest(quint32 requestID)
	{
		return sendRequest(requestID, nullptr, 0);
	}

	bool Client::sendRequest(quint32 requestID, const QByteArray& requestData)
	{
		return sendRequest(requestID, requestData.constData(), static_cast<quint32>(requestData.size()));
	}

	bool Client::sendRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
	{
		AUTO_LOCK(m_mutex);

		if (isClearToSendRequest() == false)
		{
			Q_ASSERT(false);
			return false;
		}

		if (m_socket == nullptr)
		{
			Q_ASSERT(false);
			return false;
		}

		addRequest();

		m_sentRequestHeader.type = Header::Type::Request;
		m_sentRequestHeader.id = requestID;
		m_sentRequestHeader.numerator = m_requestNumerator;
		m_sentRequestHeader.dataSize = requestDataSize;
		m_sentRequestHeader.calcCRC();

		m_requestNumerator++;

		qint64 written = socketWrite(m_sentRequestHeader);

		if (written == -1)
		{
			qDebug() << qPrintable(QString("Socket write error: %1").arg(m_socket->errorString()));
			return false;
		}

		if (written < static_cast<qint64>(sizeof(m_sentRequestHeader)))
		{
			Q_ASSERT(false);
			return false;
		}

		if (requestDataSize > 0)
		{
			if (requestData == nullptr)
			{
				Q_ASSERT(false);
				return false;
			}

			written = socketWrite(requestData, requestDataSize);

			if (written == -1)
			{
				qDebug() << qPrintable(QString("Socket write error: %1").arg(m_socket->errorString()));
				return false;
			}

			if (written < requestDataSize)
			{
				Q_ASSERT(false);
				return false;
			}
		}

		m_socket->flush();

		startTimeoutTimer();

		m_noRequestsTimeout = 0;

		m_clientState = ClientState::WaitingForReply;
		m_readState = ReadState::WaitingForHeader;

		return true;
	}

	bool Client::sendRequest(quint32 requestID, google::protobuf::Message& protobufMessage)
	{
		int messageSize = static_cast<int>(protobufMessage.ByteSizeLong());

		if (messageSize > TCP_MAX_DATA_SIZE)
		{
			return false;
		}

		if (m_protobufBuffer == nullptr)
		{
			m_protobufBuffer = new char [TCP_MAX_DATA_SIZE];

			Q_ASSERT(m_protobufBuffer != nullptr);
		}

		protobufMessage.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(m_protobufBuffer));

		return sendRequest(requestID, m_protobufBuffer, messageSize);
	}

	void Client::enableClientAliveRequest(bool enable)
	{
		AUTO_LOCK(m_mutex);

		m_enableClientAliveRequest = enable;
	}

	void Client::encrypted()
	{
		logMessage(QString("Connection encrypted"));

		sendIntroduceMyselfRequest();
	}

	void Client::sslErrors(const QList<QSslError>& errors)
	{
		int notIgnoredErrCount = 0;

		for(const QSslError& err : errors)
		{
			if (isIgnoredSslError(err) == false)
			{
				notIgnoredErrCount++;
			}
		}

		if (notIgnoredErrCount > 0)
		{
			logError(QString("Ssl errors occured on connection:"));

			for(const QSslError& err : errors)
			{
				logError(QString("(%1) %2").
							  arg(static_cast<int>(err.error())).arg(err.errorString()));
			}
		}
	}

	void Client::errorOccurred(QAbstractSocket::SocketError socketError)
	{
		logError(QString("Connection error occured: (%1) %2").
					arg(static_cast<int>(socketError)).arg(m_socket->errorString()));
	}

	void Client::handshakeInterruptedOnError(const QSslError& error)
	{
		logError(QString("Connection handshake interrupted on error: (%1) %2").
						arg(static_cast<int>(error.error())).arg(error.errorString()));
	}

	void Client::modeChanged(QSslSocket::SslMode mode)
	{
		logMessage(QString("Connection mode changed to %1").
						arg(sslModeStr(mode)));
	}

	void Client::peerVerifyError(const QSslError&error)
	{
		if (isIgnoredSslError(error) == false)
		{
			logError(QString("Connection peer verify error: (%1) %2").
						arg(static_cast<int>(error.error())).arg(error.errorString()));
		}
	}

	void Client::preSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator* authenticator)
	{
		Q_UNUSED(authenticator);

		logWarning(QString("Connection preSharedKeyAuthenticationRequired"));
	}

	void Client::onTimeoutTimer()
	{
		qDebug() << "Tcp::Client::onTimeoutTimer()";
		onReplyTimeout();
		closeConnection();
	}

	void Client::slot_onPeriodicTimer()
	{
		AUTO_LOCK(m_mutex);

		if (isConnected() == false)
		{
			m_connectTimeout++;

			if (m_connectTimeout >= 6 /* 6 * 0.5 sec == 3 sec */)
			{
				autoSwitchServer();
				createSocket();
				connectToServer();

				m_connectTimeout = 0;
				m_noRequestsTimeout = 0;
			}

			return;
		}

		//

		m_noRequestsTimeout++;

		if (m_noRequestsTimeout >= 6  /* 6 * 0.5 sec == 3 sec */)
		{
			bool res = sendClientAliveRequest();

			if (res == true)
			{
				m_noRequestsTimeout = 0;
			}
		}
	}

	void Client::autoSwitchServer()
	{
		AUTO_LOCK(m_mutex)

		if (m_autoSwitchServer == true)
		{
			selectNextValidServer();
		}
	}

	void Client::selectFirstValidServer()
	{
		m_selectedServerIndex = 1;		// to begin from server 0

		selectNextValidServer();
	}

	void Client::selectNextValidServer()
	{
		bool server0IsSet = m_serversAddressPort[0].isSet();
		bool server1IsSet = m_serversAddressPort[1].isSet();

		if (server0IsSet == true && server1IsSet == true)
		{
			// both servers is valid
			//
			if (m_selectedServerIndex == 0)
			{
				m_selectedServerIndex = 1;
			}
			else
			{
				m_selectedServerIndex = 0;
			}
		}
		else
		{
			// one or both of servers is not valid
			//
			if (server0IsSet == true)
			{
				m_selectedServerIndex = 0;
			}
			else
			{
				if (server1IsSet == true)
				{
					m_selectedServerIndex = 1;
				}
				else
				{
					m_selectedServerIndex = 0;		// addresses of both servers isn't set
				}
			}
		}

		m_selectedServer = m_serversAddressPort[m_selectedServerIndex];
	}

	void Client::connectToServer()
	{
		AUTO_LOCK(m_mutex);

		if (m_socket == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		onTryConnectToServer(m_selectedServer);

		if (m_selectedServer.isSet() == true)
		{
			m_socket->connectToHost(m_selectedServer.address(), m_selectedServer.port());
		}
	}

	void Client::onThreadStarted()
	{
		onClientThreadStarted();

		SocketWorker::onThreadStarted();

		connect(m_socket, &QSslSocket::encrypted, this, &Client::encrypted);
		connect(m_socket, &QSslSocket::sslErrors, this, &Client::sslErrors);
		connect(m_socket, &QSslSocket::errorOccurred, this, &Client::errorOccurred);
		connect(m_socket, &QSslSocket::handshakeInterruptedOnError, this, &Client::handshakeInterruptedOnError);
		connect(m_socket, &QSslSocket::modeChanged, this, &Client::modeChanged);
		connect(m_socket, &QSslSocket::peerVerifyError, this, &Client::peerVerifyError);
		connect(m_socket, &QSslSocket::preSharedKeyAuthenticationRequired, this, &Client::preSharedKeyAuthenticationRequired);

		connect(&m_periodicTimer, &QTimer::timeout, this, &Client::slot_onPeriodicTimer);

		m_periodicTimer.setInterval(TCP_PERIODIC_TIMER_INTERVAL);
		m_periodicTimer.start();

		connectToServer();
	}

	void Client::onThreadFinished()
	{
		onClientThreadFinished();

		if (m_socket != nullptr)
		{
			m_socket->disconnectFromHost();
			m_socket->close();
		}
		else
		{
			Q_ASSERT(false);
		}

		SocketWorker::onThreadFinished();
	}

	void Client::onHeaderAndDataReady()
	{
		if (m_clientState != ClientState::WaitingForReply)
		{
			Q_ASSERT(false);
			closeConnection();
			return;
		}

		stopTimeoutTimer();

		if (m_header.id != m_sentRequestHeader.id ||
			m_header.numerator != m_sentRequestHeader.numerator)
		{
			Q_ASSERT(false);
			closeConnection();
			return;
		}

		switch(m_header.type)
		{
		case Header::Type::Ack:
			onAck(m_header.id, m_receiveDataBuffer, m_header.dataSize);

			m_readState = ReadState::WaitingForHeader;

			startTimeoutTimer();

			break;

		case Header::Type::Reply:
			initReadStatusVariables();

			addReply();

			switch(m_header.id)
			{
			case RQID_SECURITY_LEVEL:
				if (processSecurityLevelReply(m_receiveDataBuffer, m_header.dataSize) == false)
				{
					closeConnection();
				}
				break;

			case RQID_INTRODUCE_MYSELF:
				if (processIntroduceMyselfReply(m_receiveDataBuffer, m_header.dataSize) == true)
				{
					onConnection();
				}
				break;

			default:
				processReply(m_header.id, m_receiveDataBuffer, m_header.dataSize);
			}

			break;

		default:
			Q_ASSERT(false);
		}
	}

	void Client::initReadStatusVariables()
	{
		m_clientState = ClientState::ClearToSendRequest;
		m_readState = ReadState::WaitingNothing;
		m_readHeaderSize = 0;
		m_readDataSize = 0;
		m_connectTimeout = 0;
	}

	bool Client::processSecurityLevelReply(const char* dataBuffer, int dataSize)
	{
		if (dataSize != sizeof(qint32))
		{
			logError(QString("Wrong format of reply on SecurityLevel request!"));
			return false;
		}

		qint32 securityLevelInt = *reinterpret_cast<const qint32*>(dataBuffer);

		if (E::contains<E::SecurityLevel>(securityLevelInt) == false)
		{
			logError(QString("Unknown socket SecurityLevel value - %1!").arg(securityLevelInt));
			return false;
		}

		m_securityLevel = static_cast<E::SecurityLevel>(securityLevelInt);

		switch(m_securityLevel)
		{
		case E::SecurityLevel::Basic:

			sendIntroduceMyselfRequest();

			return true;

		case E::SecurityLevel::Encoded:
			{
				QList<QSslError> errList;

				for(auto errCode : m_ignoredErrors)
				{
					errList.append(QSslError(errCode));
				}

				m_socket->ignoreSslErrors(errList);
			}

		case E::SecurityLevel::SSL:

			if (loadCertificate(true) == false)
			{
				return false;
			}

			m_socket->setPeerVerifyMode(m_securityLevel == E::SecurityLevel::Encoded ?
											QSslSocket::QueryPeer :
											QSslSocket::VerifyPeer);

			m_socket->startClientEncryption();

			// sendIntroduceMyselfRequest() will be called in encrypted() slot after SSL handshake done
			//

			return true;

		default:
			Q_ASSERT(false);
		}

		return false;
	}

	bool Client::processIntroduceMyselfReply(const char* dataBuffer, int dataSize)
	{
		Network::IntroduceMyselfReply imr;

		bool result = imr.ParseFromArray(dataBuffer, dataSize);

		if (result == false)
		{
			Q_ASSERT(false);
			return false;
		}

		m_stateMutex.lock();

		m_state.connectedSoftwareInfo.serializeFrom(imr.serversoftwareinfo());

		m_stateMutex.unlock();

		result = false;

		m_setConnectionResult = static_cast<SetConnectionResult>(imr.setconnectionerror());

		switch(m_setConnectionResult)
		{
		case SetConnectionResult::Ok:
			result = true;
			break;

		case SetConnectionResult::UnknownClientID:

			if (m_enableSignalUnknownClientID == true)
			{
				emit signal_unknownClientID(QString::fromStdString(imr.errormsg()));
				m_enableSignalUnknownClientID = false;
			}

			break;

		case SetConnectionResult::WrongClientHostname:

			if (m_enableSignalWrongClientHostname == true)
			{
				emit signal_wrongClientHostname(QString::fromStdString(imr.errormsg()));
				m_enableSignalWrongClientHostname = false;
			}
			break;

		default:
			Q_ASSERT(false);
		}

		return result;
	}

	void Client::sendIntroduceMyselfRequest()
	{
		SoftwareInfo locSoftwareInfo = localSoftwareInfo();

		Network::IntroduceMyselfRequest imr;

		locSoftwareInfo.serializeTo(imr.mutable_clientsoftwareinfo());

		sendRequest(RQID_INTRODUCE_MYSELF, imr);
	}

	bool Client::sendClientAliveRequest()
	{
		AUTO_LOCK(m_mutex);

		if (m_enableClientAliveRequest == false)
		{
			return true;
		}

		if (isClearToSendRequest() == false)
		{
			return false;
		}

		if (m_socket == nullptr)
		{
			return false;
		}

		// Request TCP_CLIENT_ALIVE is not require reply
		// so, the state of socket will not change
		//
		Header clientAlive;

		clientAlive.type = Header::Type::Request;
		clientAlive.id = TCP_CLIENT_ALIVE;
		clientAlive.numerator = m_requestNumerator;
		clientAlive.dataSize = 0;
		clientAlive.calcCRC();

		m_requestNumerator++;

		qint64 written = socketWrite(clientAlive);

		if (written == -1)
		{
			qDebug() << qPrintable(QString("Socket write error: %1").arg(m_socket->errorString()));
			return false;
		}

		if (written < static_cast<qint64>(sizeof(m_sentRequestHeader)))
		{
			Q_ASSERT(false);
			return false;
		}

//		qDebug() << "Tcp::Client::sendClientAliveRequest()";

		return true;
	}

	bool Client::isIgnoredSslError(const QSslError& sslErr) const
	{
		switch(m_securityLevel)
		{
		case E::SecurityLevel::Basic:
			Q_ASSERT(false);				// in this mode no SSL errors should be detected
			return true;

		case E::SecurityLevel::Encoded:
			return m_ignoredErrors.contains(sslErr.error());

		case E::SecurityLevel::SSL:
			break;

		default:
			Q_ASSERT(false);
		}

		return false;
	}

}
