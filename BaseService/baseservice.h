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
    quint16 m_servicePort;
    UdpSocketThread* m_baseSocketThread;

    BaseServiceController* m_baseServiceController;

public:

    BaseServiceWorker(BaseServiceController* baseServiceController, quint16 port);
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
    QThread m_baseWorkerThread;
    QThread m_mainFunctionThread;

    qint64 m_serviceStartTime;
    qint64 m_mainFunctionStartTime;

    MainFunctionState m_mainFunctionState;

    quint32 m_majorVersion;
    quint32 m_minorVersion;
    quint32 m_buildNo;

public:
    BaseServiceController(quint16 por);
    virtual ~BaseServiceController();
};


// BaseService class
//

class BaseService : public QtService<QCoreApplication>
{
private:
    BaseServiceController* m_baseServiceController;
    quint16 m_port;

public:
    BaseService(int argc, char ** argv, const QString & name, quint16 port);
    virtual ~BaseService();

protected:
    void start() override;
    void stop() override;
};
