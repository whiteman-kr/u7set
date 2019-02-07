#include "AppDataServiceClient.h"
#include "../../lib/SoftwareInfo.h"
#include <QTcpSocket>
#include "../../lib/HostAddressPort.h"
#include "../../lib/Tcp.h"
#include "TestUtils.h"


AppDataServiceClient::AppDataServiceClient(Network::GetAppDataSourcesStatesReply& dataSourceStateMessage,
										   const SoftwareInfo& softwareInfo,
										   const HostAddressPort& serverAddressPort) :
	m_dataSourceStateMessage(dataSourceStateMessage),
	m_softwareInfo(softwareInfo),
	m_serverAddressPort(serverAddressPort)
{
	m_tcpSocket = new QTcpSocket;

	m_tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, QVariant(1));

	QObject::connect(m_tcpSocket, &QTcpSocket::readyRead, &m_signalWaiter, &QEventLoop::quit);
}

AppDataServiceClient::~AppDataServiceClient()
{
	delete m_tcpSocket;

	if (m_buffer != nullptr)
	{
		delete m_buffer;
	}
}

void AppDataServiceClient::sendRequestAndWaitForResponse(quint32 requestID, bool& result)
{
	sendRequestAndWaitForResponse(requestID, nullptr, 0, result);
}

void AppDataServiceClient::sendRequestAndWaitForResponse(quint32 requestID, google::protobuf::Message& protobufMessage, bool& result)
{
	int messageSize = protobufMessage.ByteSize();

	VERIFY_STATEMENT_STR(messageSize <= Tcp::TCP_MAX_DATA_SIZE,
						 QString("Request %1 has message size %2 while limit is %3")
						 .arg(requestID)
						 .arg(messageSize)
						 .arg(Tcp::TCP_MAX_DATA_SIZE));

	if (m_buffer == nullptr)
	{
		m_buffer = new char[Tcp::TCP_MAX_DATA_SIZE];
	}

	protobufMessage.SerializeWithCachedSizesToArray(reinterpret_cast<google::protobuf::uint8*>(m_buffer));

	sendRequestAndWaitForResponse(requestID, m_buffer, messageSize, result);
}

void AppDataServiceClient::sendRequestAndWaitForResponse(quint32 requestID, const char* requestData, quint32 requestDataSize, bool& result)
{
	ensureConnectedToService(result);
	VERIFY_RESULT("Trying to send request while not connected to service");

	// Sending header
	//
	m_sendRequestHeader.type = Tcp::SocketWorker::Header::Type::Request;
	m_sendRequestHeader.id = requestID;
	m_sendRequestHeader.numerator = m_requestNumerator;
	m_sendRequestHeader.dataSize = requestDataSize;
	m_sendRequestHeader.calcCRC();

	m_requestNumerator++;

	socketWrite(reinterpret_cast<char*>(&m_sendRequestHeader), sizeof(m_sendRequestHeader), result);
	VERIFY_RESULT("Got error while sending request header");

	// Sending data
	//
	if (requestDataSize > 0)
	{
		VERIFY_STATEMENT(requestData != nullptr, "Null data passed to request while data size is more than zero");

		socketWrite(requestData, requestDataSize, result);
		VERIFY_RESULT("Got error while sending request data");
	}

	// Reading header
	//
	socketRead(reinterpret_cast<char*>(&m_receiveReplyHeader), sizeof(m_receiveReplyHeader), result);
	VERIFY_RESULT("Got error while reading answer header");

	VERIFY_STATEMENT(m_receiveReplyHeader.checkCRC(), "Received header with CRC error");

	VERIFY_STATEMENT(m_receiveReplyHeader.id == m_sendRequestHeader.id, "Reply id mismatch");

	VERIFY_STATEMENT(m_receiveReplyHeader.numerator == m_sendRequestHeader.numerator, "Reply numerator mismatch");

	VERIFY_STATEMENT(m_receiveReplyHeader.type == Tcp::SocketWorker::Header::Type::Reply, "Reply type is not reply");

	// Reading data
	//
	if (m_receiveReplyHeader.dataSize > 0)
	{
		VERIFY_STATEMENT_STR(m_receiveReplyHeader.dataSize <= Tcp::TCP_MAX_DATA_SIZE,
							 QString("Datasize in received header is %1 while it must be less than %2")
							 .arg(m_receiveReplyHeader.dataSize)
							 .arg(Tcp::TCP_MAX_DATA_SIZE));

		socketRead(m_buffer, m_receiveReplyHeader.dataSize, result);
		VERIFY_RESULT("Got error while reading answer data");
	}

	processData(result);
	VERIFY_RESULT("Processing data error");
}

void AppDataServiceClient::initDataSourceArray(QVector<DataSource>& dataSourceArray)
{
	int sourcesQuantity = m_dataSourceInfoMessage.datasourceinfo_size();

	dataSourceArray.clear();
	dataSourceArray.reserve(sourcesQuantity);

	for(int i = 0; i < sourcesQuantity; i++)
	{
		DataSource nextSource;
		nextSource.setInfo(m_dataSourceInfoMessage.datasourceinfo(i));

		bool alreadyExists = false;
		for (int j = 0; j < dataSourceArray.size(); j++)
		{
			if (dataSourceArray[j].ID() == nextSource.ID())
			{
				alreadyExists = true;
				break;
			}
		}

		if (alreadyExists == false)
		{
			dataSourceArray.push_back(nextSource);
		}
	}

	dataSourceArray.squeeze();
}

void AppDataServiceClient::ensureConnectedToService(bool& result)
{
	if (m_tcpSocket->state() == QAbstractSocket::ConnectedState)
	{
		result = true;
		return;
	}

	m_tcpSocket->connectToHost(m_serverAddressPort.address(), m_serverAddressPort.port());
	VERIFY_STATEMENT_STR(m_tcpSocket->waitForConnected(Tcp::TCP_ON_CLIENT_REQUEST_REPLY_TIMEOUT),
						 "Could not make tcp connection to " + m_serverAddressPort.addressPortStr());

	Network::SoftwareInfo message;

	m_softwareInfo.serializeTo(&message);

	sendRequestAndWaitForResponse(RQID_INTRODUCE_MYSELF, message, result);
	VERIFY_RESULT("Failed to request RQID_INTRODUCE_MYSELF");
}

void AppDataServiceClient::socketWrite(const char* data, quint32 dataSize, bool& result)
{
	qint64 writtenQuantity = m_tcpSocket->write(data, dataSize);

	VERIFY_STATEMENT_STR(writtenQuantity != -1, "Socket write error: " + m_tcpSocket->errorString());

	VERIFY_STATEMENT_STR(writtenQuantity == static_cast<int>(dataSize),
						 QString("Socket sent %1 bytes instead of %2")
						 .arg(writtenQuantity)
						 .arg(dataSize));

	QTimer::singleShot(Tcp::TCP_BYTES_WRITTEN_TIMEOUT, &m_signalWaiter, SLOT(quit()));
	m_signalWaiter.exec();

	VERIFY_STATEMENT(m_tcpSocket->bytesToWrite() == 0, "Socket write timeout");
}

void AppDataServiceClient::socketRead(char* data, quint32 dataSize, bool& result)
{
	quint32 totalBytesRead = 0;

	while (totalBytesRead < dataSize)
	{
		QTimer::singleShot(Tcp::TCP_ON_CLIENT_REQUEST_REPLY_TIMEOUT, &m_signalWaiter, SLOT(quit()));
		m_signalWaiter.exec();

		qint64 bytesAvailable = m_tcpSocket->bytesAvailable();

		VERIFY_STATEMENT_STR(totalBytesRead < dataSize && bytesAvailable > 0, "Socket read error:" + m_tcpSocket->errorString());

		quint32 bytesToRead = std::min(dataSize - totalBytesRead, static_cast<quint32>(bytesAvailable));

		qint64 bytesRead = m_tcpSocket->read(data, bytesToRead);

		VERIFY_STATEMENT_STR(bytesRead == bytesToRead, "Socket read error: %1" + m_tcpSocket->errorString());

		totalBytesRead += bytesRead;
	}
}

void AppDataServiceClient::processData(bool& result)
{
	switch (m_receiveReplyHeader.id)
	{
	case RQID_INTRODUCE_MYSELF:
	{
		Network::SoftwareInfo message;

		parseMessageLoggingErrors(message, result);
		VERIFY_RESULT("Got error while parsing RQID_INTRODUCE_MYSELF message");

		m_serviceSoftwareInfo.serializeFrom(message);

		VERIFY_STATEMENT_STR(m_serviceSoftwareInfo.softwareType() == E::SoftwareType::AppDataService,
							 "TCP Connection established to wrong software: " +
							 E::valueToString<E::SoftwareType>(m_serviceSoftwareInfo.softwareType()));

		break;
	}
	case ADS_GET_APP_DATA_SOURCES_STATES:
		parseMessageLoggingErrors(m_dataSourceStateMessage, result);
		VERIFY_RESULT("Got error while parsing ADS_GET_APP_DATA_SOURCES_STATES message");

		break;
	case ADS_GET_APP_DATA_SOURCES_INFO:
		parseMessageLoggingErrors(m_dataSourceInfoMessage, result);
		VERIFY_RESULT("Got error while parsing ADS_GET_APP_DATA_SOURCES_INFO message");

		break;
	default:
		result = false;
		FAIL_STR(QString("Unknown Reply header ID: %1").arg(m_receiveReplyHeader.id));
	}
}

void AppDataServiceClient::parseMessageLoggingErrors(google::protobuf::Message& protobufMessage, bool& result)
{
	VERIFY_STATEMENT(protobufMessage.ParsePartialFromArray(m_buffer, m_receiveReplyHeader.dataSize), "Message has invalid format");

	VERIFY_STATEMENT_STR(protobufMessage.IsInitialized(),
			   "Message has missing required fields: " +
			   QString::fromStdString(protobufMessage.InitializationErrorString()));
}

