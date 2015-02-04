#pragma once

#include <QUdpSocket>
#include <QThread>

#include "../include/SocketIO.h"


class FscDataAcquisitionSocket : public QUdpSocket
{
public:
	FscDataAcquisitionSocket(HostAddressPort fscDataAcquisitionAddressPort);
	~FscDataAcquisitionSocket();
};


class FscDataAcquisitionThread : public QObject
{
	Q_OBJECT

private:
	QThread m_thread;

public:
	FscDataAcquisitionThread(HostAddressPort fscDataAcquisitionAddressPort, QObject *parent = 0);
	~FscDataAcquisitionThread();

signals:

public slots:

};


