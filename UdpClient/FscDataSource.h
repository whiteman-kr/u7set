#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QThread>
#include <QTimer>
#include "../lib/SocketIO.h"


class FscDataSourceWorker : public QObject
{
	Q_OBJECT

private:
	QUdpSocket m_socket;
	QTimer m_timer;

	HostAddressPort m_hostAddressPort;
	QHostAddress m_sourceAddress;

	quint32 m_interval = 100;
	quint32 m_partCount = 3;
	quint32 m_packetNo = 0;

public:
	FscDataSourceWorker(HostAddressPort hostAddressPort, QHostAddress sourceAddress, quint32 interval, quint32 partCount, QObject *parent = 0);

public slots:

	void init();
	void onTimer();
};


class FscDataSource : public QObject
{
	Q_OBJECT

private:
	QThread m_thread;

public:
	FscDataSource(HostAddressPort hostAddressPort, QHostAddress sourceAddress, quint32 interval, quint32 partCount, QObject *parent = 0);
	~FscDataSource();
};

