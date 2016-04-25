#pragma once

#include <QTimer>
#include <QUdpSocket>

#include "../include/DataSource.h"
#include "../include/DataProtocols.h"
#include "../include/OrderedHash.h"
#include "../include/SimpleThread.h"


class DataChannel : public SimpleThreadWorker
{
private:
	DataSource::DataType m_dataType;
	int m_channel = 0;
	HostAddressPort m_dataReceivingIP;

	HashedVector<quint32, DataSource*> m_dataSources;

	HashedVector<quint32, quint32> m_unknownDataSources;

	QTimer m_timer;

	QUdpSocket* m_socket = nullptr;
	bool m_socketBound = false;

	RupFrame m_rupFrame;

	virtual void onThreadStarted();
	virtual void onThreadFinished();

	virtual void onTimer();
	//virtual void onSocklet

	void createAndBindSocket();
	void closeSocket();
	void onSocketReadyRead();

public:
	explicit DataChannel(int channel, DataSource::DataType dataType, const HostAddressPort& dataReceivingIP);
	virtual ~DataChannel();

	void clear();

	void addDataSource(DataSource* dataSource);

signals:

public slots:
};


class AppDataChannel : public DataChannel
{
	Q_OBJECT
public:
	AppDataChannel(int channel, const HostAddressPort& dataReceivingIP);

signals:

public slots:
};


class DiagDataChannel : public DataChannel
{
	Q_OBJECT
public:
	DiagDataChannel(int channel, const HostAddressPort& dataReceivingIP);

signals:

public slots:
};


class DataChannelThread : public SimpleThread
{
private:
	DataChannel* m_dataChannel = nullptr;

public:
	DataChannelThread(int channel, DataSource::DataType dataType, const HostAddressPort& dataRecievingIP);

	void addDataSource(DataSource* dataSource);
};


