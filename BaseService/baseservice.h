#pragma once

#include <QObject>
#include <QThread>
#include <QDebug>
#include "../qtservice/src/qtservice.h"
#include "../include/UdpSocket.h"


class MainWorker : public QObject
{
    Q_OBJECT

private:
    enum ServiceMainFunctionState
    {
        Stopped,
        Starts,
        Work,
        Stops
    };

    quint16 m_servicePort;
    UdpSocketThread* m_baseSocketThread;

    ServiceMainFunctionState m_serviceMainFunctionState;

public:

    MainWorker(quint16 port);
    virtual ~MainWorker();

    virtual void mainWorkerThreadStarted() {}
    virtual void mainWorkerThreadFinished() {}

signals:
    void ackBaseRequest(UdpRequest request);

public slots:
    void onMainWorkerThreadStarted();
    void onMainWorkerThreadFinished();

    void onBaseRequest(UdpRequest request);
};


class MainWorkerController : public QObject
{
    Q_OBJECT

public:
    MainWorkerController(quint16 port);
    virtual ~MainWorkerController();


private:
    QThread m_mainWorkerThread;
};



class BaseService : public QtService<QCoreApplication>
{
public:
    BaseService(int argc, char ** argv, const QString & name, quint16 port);
    virtual ~BaseService();

protected:
    void start() override;
    void stop() override;

signals:

public slots:

private:
    MainWorkerController* m_mainWorkerController;
    quint16 m_servicePort;
};
