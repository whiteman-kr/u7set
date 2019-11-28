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

	const QThread* m_thisThread = nullptr;

	//

	QUdpSocket* m_socket = nullptr;

	HashedVector<quint32, quint32> m_unknownAppDataSourcesIP;

	//

	int m_receivingRate = 0;				// bytes per second
	int m_udpReceivingRate = 0;				// UDP datagrams per second
	int m_rupFramesReceivingRate = 0;		// RUP frames per second
	qint64 m_rupFramesCount = 0;
	qint64 m_simFramesCount = 0;

	qint64 m_errDatagramSize = 0;
	qint64 m_errSimVersion = 0;
	qint64 m_errUnknownAppDataSourceIP = 0;
	qint64 m_errRupFrameCRC = 0;

	//

	int m_receivedPerSecond = 0;
	int m_udpReceivedPerSecond = 0;
	int m_rupFramesReceivedPerSecond = 0;
};
