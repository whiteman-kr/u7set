#pragma once

#include <QTimer>
#include <QUdpSocket>

#include "../include/DataSource.h"
#include "../include/DataProtocols.h"
#include "../include/OrderedHash.h"
#include "../include/SimpleThread.h"
#include "../include/Queue.h"
#include "../include/Signal.h"


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

	Queue<RupData> m_rupDataQueue;

	virtual void onThreadStarted();
	virtual void onThreadFinished();

	virtual void onTimer();

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



class DataChannelThread : public SimpleThread
{
private:
	DataChannel* m_dataChannel = nullptr;

public:
	DataChannelThread();

	void addDataSource(DataSource* dataSource);
};


