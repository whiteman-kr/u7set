#pragma once

//#include "../lib/DataChannel.h"

#include "../lib/SimpleThread.h"
#include "../lib/AppDataSource.h"
#include "../lib/CircularLogger.h"

//
// AppDataReceiver is receives RUP datagrams and push it in AppDataSource's queues
//

class AppDataReceiver : public SimpleThreadWorker
{
public:
	AppDataReceiver(const HostAddressPort& dataReceivingIP,
					const AppDataSourcesIP& appDataSourcesIP,
					CircularLoggerShared log);

	virtual ~AppDataReceiver();

signals:
	void rupFrameIsReceived(quint32 appDataSourceIP);

private:
	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

	void createAndBindSocket();
	void closeSocket();

private slots:	
	void onTimer1s();
	void onSocketReadyRead();

private:
	HostAddressPort m_dataReceivingIP;
	const AppDataSourcesIP& m_appDataSourcesIP;
	CircularLoggerShared m_log;

	//

	QTimer m_timer1s;

	QUdpSocket* m_socket = nullptr;
	bool m_socketBound = false;

	HashedVector<quint32, quint32> m_unknownAppDataSourcesIP;

	int m_receivedFramesCount = 0;
};


class AppDataReceiverThread : public SimpleThread
{
public:
	AppDataReceiverThread(const HostAddressPort& dataRecievingIP,
						  const AppDataSourcesIP& appDataSourcesIP,
						  CircularLoggerShared log);
};

