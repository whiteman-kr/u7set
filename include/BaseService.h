#pragma once

#include <QObject>
#include <QThread>
#include <QDebug>
#include "../qtservice/src/qtservice.h"
#include "../include/UdpSocket.h"


class BaseServiceController;

struct ServiceInfo
{
    quint32 serviceType;                // RQSTP_* constants
    quint32 majorVersion;
    quint32 minorVersion;
    quint32 buildNo;
    quint32 crc;
    quint32 serviceUptime;
    quint32 mainFunctionSate;           // SS_MF_* constants
    quint32 mainFunctionUptime;
};


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

public slots:
    void onBaseServiceWorkerThreadStarted();
    void onBaseServiceWorkerThreadFinished();

    void onBaseRequest(UdpRequest request);
};


// BaseServiceController class
//


class BaseServiceController : public QObject
{
    Q_OBJECT

public:
    enum MainFunctionState
    {
        Stopped,
        Starts,
        Work,
        Stops
    };

private:
    QMutex m_mutex;

    QThread m_baseWorkerThread;
    QThread m_mainFunctionThread;

    int m_serviceType;

    qint64 m_serviceStartTime;
    qint64 m_mainFunctionStartTime;

    MainFunctionState m_mainFunctionState;

    quint32 m_majorVersion;
    quint32 m_minorVersion;
    quint32 m_buildNo;
	quint32 m_crc;

public:
    BaseServiceController(int serviceType);
    virtual ~BaseServiceController();

    void getServiceInfo(ServiceInfo& serviceInfo);
};


