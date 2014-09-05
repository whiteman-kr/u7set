#pragma once

#include <QObject>
#include <QThread>
#include <QDebug>
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

    UdpSocketThread* m_baseSocketThread;

    BaseServiceController* m_baseServiceController;

public:

    BaseServiceWorker(BaseServiceController* baseServiceController, int serviceType);
    virtual ~BaseServiceWorker();

    virtual void baseServiceWorkerThreadStarted() {}
    virtual void baseServiceWorkerThreadFinished() {}

signals:
    void ackBaseRequest(UdpRequest request);

	void startMainFunction();
	void stopMainFunction();
	void restartMainFunction();

public slots:
    void onBaseServiceWorkerThreadStarted();
    void onBaseServiceWorkerThreadFinished();

    void onBaseRequest(UdpRequest request);
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

	virtual void onThreadStarted() { qDebug() << "Called MainFunctionWorker::onThreadStarted"; }
	virtual void onThreadFinished() { qDebug() << "Called MainFunctionWorker::onThreadFinished"; }

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
		Stopped = SS_MF_STOPPED,
		Starts = SS_MF_STARTS,
		Work = SS_MF_WORK,
		Stops = SS_MF_STOPS
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

public slots:
	void stopMainFunction();
	void startMainFunction();
	void restartMainFunction();

private slots:
	void onTimer500ms();

	void onMainFunctionWork();
	void onMainFunctionStopped();

public:
    BaseServiceController(int serviceType);
    virtual ~BaseServiceController();

	void getServiceInfo(ServiceInformation& serviceInfo);
};


