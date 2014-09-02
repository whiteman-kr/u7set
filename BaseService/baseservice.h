#pragma once

#include <QObject>
#include <QThread>
#include <QDebug>
#include "../qtservice/src/qtservice.h"
#include "../include/UdpSocket.h"


class MainWorker : public QObject
{
    Q_OBJECT

public:
    enum ServiceMainFunctionState
    {
        Stopped,
        Starts,
        Work,
        Stops
    };

    MainWorker();
    virtual ~MainWorker();

public slots:
    void onMainWorkerThreadStarted();
    void onMainWorkerThreadFinished();

private:
    UdpServerSocket* m_baseUdpServerSocket;

    ServiceMainFunctionState m_serviceMainFunctionState;
};


class MainWorkerController : public QObject
{
    Q_OBJECT

public:
    MainWorkerController();
    virtual ~MainWorkerController();


private:
    QThread m_mainWorkerThread;
};



class BaseService : public QtService<QCoreApplication>
{
public:
    BaseService(int argc, char ** argv, const QString & name);
    virtual ~BaseService();

    virtual void getBindToAddress(QHostAddress& bindToAddress, quint16 &port);

protected:
    void start() override;
    void stop() override;

signals:

public slots:

private:
    MainWorkerController* m_mainWorkerController;
};
