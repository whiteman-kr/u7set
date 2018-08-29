#ifndef APPDATASERVICECLIENT_H
#define APPDATASERVICECLIENT_H

#include <qglobal.h>
#include "../../Proto/network.pb.h"
#include "../../lib/Tcp.h"

class AppDataServiceClient
{
public:
	AppDataServiceClient(Network::GetAppDataSourcesStatesReply& dataSourceStateMessage,
						 const SoftwareInfo& softwareInfo,
						 const HostAddressPort& serverAddressPort);

	virtual ~AppDataServiceClient();

	bool sendRequestAndWaitForResponse(quint32 requestID, QString& error);
	bool sendRequestAndWaitForResponse(quint32 requestID, google::protobuf::Message& protobufMessage, QString& error);
	bool sendRequestAndWaitForResponse(quint32 requestID, const char* requestData, quint32 requestDataSize, QString& error);

private:
	bool ensureConnectedToService(QString& error);
	bool socketWrite(const char* data, quint32 dataSize, QString& error);
	bool socketRead(char* data, quint32 dataSize, QString& error);
	bool processData(QString& error);
	bool parseMessageLoggingErrors(google::protobuf::Message& protobufMessage, QString& error);

	Network::GetAppDataSourcesStatesReply& m_dataSourceStateMessage;
	SoftwareInfo m_softwareInfo;
	SoftwareInfo m_serviceSoftwareInfo;
	const HostAddressPort& m_serverAddressPort;
	QTcpSocket* m_tcpSocket = nullptr;
	char* m_buffer = nullptr;
	Tcp::SocketWorker::Header m_sendRequestHeader;
	Tcp::SocketWorker::Header m_receiveReplyHeader;
	quint32 m_requestNumerator = 1;
	QEventLoop signalWaiter;
};

#endif // APPDATASERVICECLIENT_H
