#ifndef APPDATASERVICECLIENT_H
#define APPDATASERVICECLIENT_H

#include <qglobal.h>
#include "../../Proto/network.pb.h"
#include "../../lib/Tcp.h"
#include "../../lib/DataSource.h"

class AppDataServiceClient
{
public:
	AppDataServiceClient(Network::GetAppDataSourcesStatesReply& dataSourceStateMessage,
						 const SoftwareInfo& softwareInfo,
						 const HostAddressPort& serverAddressPort);

	virtual ~AppDataServiceClient();

	void sendRequestAndWaitForResponse(quint32 requestID, bool& result);
	void sendRequestAndWaitForResponse(quint32 requestID, google::protobuf::Message& protobufMessage, bool& result);
	void sendRequestAndWaitForResponse(quint32 requestID, const char* requestData, quint32 requestDataSize, bool& result);

	void initDataSourceArray(QVector<DataSource>& dataSourceArray);

private:
	void ensureConnectedToService(bool& result);
	void socketWrite(const char* data, quint32 dataSize, bool& result);
	void socketRead(char* data, quint32 dataSize, bool& result);
	void processData(bool& result);
	void parseMessageLoggingErrors(google::protobuf::Message& protobufMessage, bool& result);

	Network::GetAppDataSourcesStatesReply& m_dataSourceStateMessage;
	Network::GetDataSourcesInfoReply m_dataSourceInfoMessage;
	SoftwareInfo m_softwareInfo;
	SoftwareInfo m_serviceSoftwareInfo;
	const HostAddressPort& m_serverAddressPort;
	QTcpSocket* m_tcpSocket = nullptr;
	char* m_buffer = nullptr;
	Tcp::SocketWorker::Header m_sendRequestHeader;
	Tcp::SocketWorker::Header m_receiveReplyHeader;
	quint32 m_requestNumerator = 1;
	QEventLoop m_signalWaiter;
};

#endif // APPDATASERVICECLIENT_H
