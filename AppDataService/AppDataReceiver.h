#pragma once

#include <QUdpSocket>

#include "../lib/SimpleThread.h"
#include "AppDataSource.h"
#include "../lib/CircularLogger.h"

//
// AppDataReceiver is receives RUP datagrams and push it in AppDataSource's queues
//

class AppDataReceiver : public SimpleThreadWorker
{
	Q_OBJECT

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
	QTimer m_shortTimer;

	QUdpSocket* m_socket = nullptr;
	bool m_socketBound = false;

	HashedVector<quint32, quint32> m_unknownAppDataSourcesIP;
	qint64 m_unknownAppDataSourcesCount = 0;

	int m_receivedFramesCount = 0;

	//

	qint64 m_simFrameCount = 0;

	qint64 m_errDatagramSize = 0;
	qint64 m_errSimVersion = 0;
	qint64 m_errUnknownAppDataSourceIP = 0;
};


class AppDataReceiverThread : public SimpleThread
{
public:
	AppDataReceiverThread(const HostAddressPort& dataRecievingIP,
						  const AppDataSourcesIP& appDataSourcesIP,
						  CircularLoggerShared log);

	AppDataReceiver* appDataReceiver() { return m_appDataReceiver; }

private:
	AppDataReceiver* m_appDataReceiver = nullptr;
};

