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
    MainWorker();
    virtual ~MainWorker();

public slots:
    void onMainWorkerThreadStarted();

private:
    UdpServerSocket* m_mainUdpServerSocket;
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

protected:
    void start() override;
    void stop() override;

signals:

public slots:

private:
    MainWorkerController* m_mainWorkerController;
};
