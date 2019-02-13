#pragma once

#include <QUdpSocket>

#include "../lib/SimpleThread.h"
#include "AppDataSource.h"
#include "../lib/CircularLogger.h"

//
// AppDataReceiver is receives RUP datagrams and push it in AppDataSource's queues
//

class AppDataReceiverThread : public RunOverrideThread
{
public:
	AppDataReceiverThread(const HostAddressPort& dataReceivingIP,
					const AppDataSourcesIP& appDataSourcesIP,
					CircularLoggerShared log);

	virtual ~AppDataReceiverThread() override;

	void fillAppDataReceiveState(Network::AppDataReceiveState* adrs);

private:
	virtual void run() override;

	bool tryCreateAndBindSocket();
	void closeSocket();

	void receivePackets();

private:
	HostAddressPort m_dataReceivingIP;
	const AppDataSourcesIP& m_appDataSourcesIP;
	CircularLoggerShared m_log;

	//

	QUdpSocket* m_socket = nullptr;

	HashedVector<quint32, quint32> m_unknownAppDataSourcesIP;

	int m_receivedFramesCount = 0;

	//

	qint64 m_simFramesCount = 0;
	qint64 m_errDatagramSize = 0;
	qint64 m_errSimVersion = 0;
	qint64 m_errUnknownAppDataSourceIP = 0;
};
