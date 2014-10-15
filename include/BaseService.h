#pragma once

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QFile>
#include "../qtservice/src/qtservice.h"
#include "../include/UdpSocket.h"


class BaseServiceController;


// BaseServiceWorker class
//

class BaseServiceWorker : public QObject
{
    Q_OBJECT

private:
    int m_serviceType;

	UdpSocketThread* m_serverSocketThread;

	UdpSocketThread* m_sendFileClientSocketThread;
	QFile* m_fileToSend;
	char* m_sendFileStartBuffer;

    BaseServiceController* m_baseServiceController;

public:

    BaseServiceWorker(BaseServiceController* baseServiceController, int serviceType);
    virtual ~BaseServiceWorker();

	virtual void threadStarted() {}
	virtual void threadFinished() {}

signals:
    void ackBaseRequest(UdpRequest request);

	void startMainFunction();
	void stopMainFunction();
	void restartMainFunction();

	void endSendFile(bool result);

	void sendFileRequest(quint32 requestID, char* requestData, quint32 requestDataSize);

private slots:
	void onSendFileRequestAck(RequestHeader header, QByteArray data);

public slots:
	void onThreadStarted();
	void onThreadFinished();

    void onBaseRequest(UdpRequest request);

	void onSendFile(QHostAddress address, quint16 port, QString fileName);
};


// MainFunctionWorker class
//

class MainFunctionWorker : public QObject
{
	Q_OBJECT

private:
	BaseServiceController* m_baseServiceController = nullptr;

public:

	MainFunctionWorker(BaseServiceController* baseServiceController);
	virtual ~MainFunctionWorker();

	virtual void threadStarted() { QThread::sleep(2); qDebug() << "Called MainFunctionWorker::threadStarted"; }
	virtual void threadFinished() { QThread::sleep(2); qDebug() << "Called MainFunctionWorker::threadFinished"; }

signals:
	void mainFunctionWork();
	void mainFunctionStopped();

public slots:
	void onThreadStartedSlot();
	void onThreadFinishedSlot();
};


// BaseServiceController class
//


class BaseServiceController : public QObject
{
    Q_OBJECT

public:
    enum MainFunctionState
    {
		stopped = SS_MF_STOPPED,
		starts = SS_MF_STARTS,
		work = SS_MF_WORK,
		stops = SS_MF_STOPS
    };

private:
    QMutex m_mutex;

    QThread m_baseWorkerThread;
    QThread m_mainFunctionThread;

	bool m_mainFunctionNeedRestart;

	bool m_mainFunctionStopped;

	int m_serviceType;

    qint64 m_serviceStartTime;
    qint64 m_mainFunctionStartTime;

    MainFunctionState m_mainFunctionState;

    quint32 m_majorVersion;
    quint32 m_minorVersion;
    quint32 m_buildNo;
	quint32 m_crc;

	QTimer m_timer500ms;

	void checkMainFunctionState();

signals:
	void sendFile(QHostAddress address, quint16 port, QString fileName);

public slots:
	void stopMainFunction();
	void startMainFunction();
	void restartMainFunction();

	virtual void onEndSendFile(bool result);

private slots:
	void onTimer500ms();

	void onMainFunctionWork();
	void onMainFunctionStopped();

public:
    BaseServiceController(int serviceType);
    virtual ~BaseServiceController();

	void getServiceInfo(ServiceInformation& serviceInfo);
};


