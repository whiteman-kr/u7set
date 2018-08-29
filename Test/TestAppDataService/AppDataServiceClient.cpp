#include "AppDataServiceClient.h"
#include "../../lib/SoftwareInfo.h"
#include <QTcpSocket>
#include "../../lib/HostAddressPort.h"
#include "../../lib/Tcp.h"


AppDataServiceClient::AppDataServiceClient(Network::GetAppDataSourcesStatesReply& dataSourceStateMessage,
										   const SoftwareInfo& softwareInfo,
										   const HostAddressPort& serverAddressPort) :
	m_dataSourceStateMessage(dataSourceStateMessage),
	m_softwareInfo(softwareInfo),
	m_serverAddressPort(serverAddressPort)
{
	m_tcpSocket = new QTcpSocket;

	m_tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, QVariant(1));

	QObject::connect(m_tcpSocket, &QTcpSocket::readyRead, &signalWaiter, &QEventLoop::quit);
}

AppDataServiceClient::~AppDataServiceClient()
{
	delete m_tcpSocket;

	if (m_buffer != nullptr)
	{
		delete m_buffer;
	}
}

bool AppDataServiceClient::sendRequestAndWaitForResponse(quint32 requestID, QString& error)
{
	return sendRequestAndWaitForResponse(requestID, nullptr, 0, error);
}

bool AppDataServiceClient::sendRequestAndWaitForResponse(quint32 requestID, google::protobuf::Message& protobufMessage, QString& error)
{
	int messageSize = protobufMessage.ByteSize();

	if (messageSize > Tcp::TCP_MAX_DATA_SIZE)
	{
		error = QString("Request %1 has message size %2 while limit is %3").arg(requestID).arg(messageSize).arg(Tcp::TCP_MAX_DATA_SIZE);
		return false;
	}

	if (m_buffer == nullptr)
	{
		m_buffer = new char[Tcp::TCP_MAX_DATA_SIZE];
	}

	protobufMessage.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(m_buffer));

	return sendRequestAndWaitForResponse(requestID, m_buffer, messageSize, error);
}

bool AppDataServiceClient::sendRequestAndWaitForResponse(quint32 requestID, const char* requestData, quint32 requestDataSize, QString& error)
{
	if (ensureConnectedToService(error) == false)
	{
		return false;
	}
	// Sending header
	//
	m_sendRequestHeader.type = Tcp::SocketWorker::Header::Type::Request;
	m_sendRequestHeader.id = requestID;
	m_sendRequestHeader.numerator = m_requestNumerator;
	m_sendRequestHeader.dataSize = requestDataSize;
	m_sendRequestHeader.calcCRC();

	m_requestNumerator++;

	if (socketWrite(reinterpret_cast<char*>(&m_sendRequestHeader), sizeof(m_sendRequestHeader), error) == false)
	{
		return false;
	}

	// Sending data
	//
	if (requestDataSize > 0)
	{
		if (requestData == nullptr)
		{
			error = "Null data passed to request while data size is more than zero";
			return false;
		}

		if (socketWrite(requestData, requestDataSize, error) == false)
		{
			return false;
		}
	}

	// Reading header
	if (socketRead(reinterpret_cast<char*>(&m_receiveReplyHeader), sizeof(m_receiveReplyHeader), error) == false)
	{
		return false;
	}

	if (m_receiveReplyHeader.checkCRC() == false)
	{
		error = "Received header with CRC error";
		return false;
	}

	if (m_receiveReplyHeader.id != m_sendRequestHeader.id)
	{
		error = "Reply id mismatch";
		return false;
	}

	if (m_receiveReplyHeader.numerator != m_sendRequestHeader.numerator)
	{
		error = "Reply numerator mismatch";
		return false;
	}
	if (m_receiveReplyHeader.type != Tcp::SocketWorker::Header::Type::Reply)
	{
		error = "Reply type is not reply";
		return false;
	}

	// Reading data
	if (m_receiveReplyHeader.dataSize > 0)
	{
		if (m_receiveReplyHeader.dataSize > Tcp::TCP_MAX_DATA_SIZE)
		{
			error = QString("Datasize in received header is %1 while it must be less than %2").arg(m_receiveReplyHeader.dataSize).arg(Tcp::TCP_MAX_DATA_SIZE);
			return false;
		}
		if (socketRead(m_buffer, m_receiveReplyHeader.dataSize, error) == false)
		{
			return false;
		}
	}

	if (processData(error) == false)
	{
		return false;
	}

	return true;
}

bool AppDataServiceClient::ensureConnectedToService(QString& error)
{
	if (m_tcpSocket->state() == QAbstractSocket::ConnectedState)
	{
		return true;
	}

	m_tcpSocket->connectToHost(m_serverAddressPort.address(), m_serverAddressPort.port());
	if (m_tcpSocket->waitForConnected(Tcp::TCP_ON_CLIENT_REQUEST_REPLY_TIMEOUT) == false)
	{
		error = "Could not make tcp connection to " + m_serverAddressPort.addressPortStr();
		return false;
	}

	Network::SoftwareInfo message;

	m_softwareInfo.serializeTo(&message);

	if (sendRequestAndWaitForResponse(RQID_INTRODUCE_MYSELF, message, error) == false)
	{
		return false;
	}
	return true;
}

bool AppDataServiceClient::socketWrite(const char* data, quint32 dataSize, QString& error)
{
	int writtenQuantity = m_tcpSocket->write(data, dataSize);

	if (writtenQuantity == -1)
	{
		error = QString("Socket write error: %1").arg(m_tcpSocket->errorString());
		return false;
	}

	if (writtenQuantity != static_cast<int>(dataSize))
	{
		error = QString("Socket sent %1 bytes instead of %2").arg(writtenQuantity).arg(dataSize);
		return false;
	}

	QTimer::singleShot(Tcp::TCP_BYTES_WRITTEN_TIMEOUT, &signalWaiter, SLOT(quit()));
	signalWaiter.exec();

	if (m_tcpSocket->bytesToWrite() > 0)
	{
		error = "Socket write timeout";
		return false;
	}

	return true;
}

bool AppDataServiceClient::socketRead(char* data, quint32 dataSize, QString& error)
{
	quint32 totalBytesRead = 0;

	while (totalBytesRead < dataSize)
	{
		QTimer::singleShot(Tcp::TCP_ON_CLIENT_REQUEST_REPLY_TIMEOUT, &signalWaiter, SLOT(quit()));
		signalWaiter.exec();

		int bytesAvailable = m_tcpSocket->bytesAvailable();
		if (bytesAvailable <= 0)
		{
			error = QString("Socket read error: %1").arg(m_tcpSocket->errorString());
			return false;
		}

		int bytesToRead = std::min(dataSize - totalBytesRead, static_cast<quint32>(bytesAvailable));

		int bytesRead = m_tcpSocket->read(data, bytesToRead);

		if (bytesRead < bytesToRead)
		{
			error = QString("Socket read error: %1").arg(m_tcpSocket->errorString());
			return false;
		}

		totalBytesRead += bytesRead;
	}

	return true;
}

bool AppDataServiceClient::processData(QString& error)
{
	switch (m_receiveReplyHeader.id)
	{
	case RQID_INTRODUCE_MYSELF:
	{
		Network::SoftwareInfo message;

		if (parseMessageLoggingErrors(message, error) == false)
		{
			error = "Reply RQID_INTRODUCE_MYSELF protobuf error: " + error;
			return false;
		}
		m_serviceSoftwareInfo.serializeFrom(message);
		if (m_serviceSoftwareInfo.softwareType() != E::SoftwareType::AppDataService)
		{
			error = "TCP Connection established to wrong software: " + E::valueToString<E::SoftwareType>(m_serviceSoftwareInfo.softwareType());
			return false;
		}
		break;
	}
	case ADS_GET_DATA_SOURCES_STATES:
		if (parseMessageLoggingErrors(m_dataSourceStateMessage, error) == false)
		{
			error = "Reply ADS_GET_DATA_SOURCES_STATES protobuf error: " + error;
			return false;
		}
		break;
	default:
		error = QString("Unknown Reply header ID: %1").arg(m_receiveReplyHeader.id);
		break;
	}
	return true;
}

bool AppDataServiceClient::parseMessageLoggingErrors(google::protobuf::Message& protobufMessage, QString& error)
{
	if (protobufMessage.ParsePartialFromArray(m_buffer, m_receiveReplyHeader.dataSize) == false)
	{
		error = "has invalid format";
		return false;
	}

	if (protobufMessage.IsInitialized() == false)
	{
		error = "has missing required fields: " + QString::fromStdString(protobufMessage.InitializationErrorString());
		return false;
	}

	return true;
}

