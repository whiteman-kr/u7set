#pragma once

#include <QTimer>
#include <QUdpSocket>

#include "../include/DataSource.h"
#include "../include/DataProtocols.h"
#include "../include/OrderedHash.h"
#include "../include/SimpleThread.h"
#include "../include/Queue.h"
#include "../include/Signal.h"


typedef Queue<RupData> RupDataQueue;


class DataChannel : public SimpleThreadWorker
{
protected:
	DataSource::DataType m_dataType;
	int m_channel = 0;
	HostAddressPort m_dataReceivingIP;

	HashedVector<quint32, DataSource*> m_dataSources;			// allocated and freed in AppDataService

	HashedVector<quint32, quint32> m_unknownDataSources;

	QTimer m_timer;

	QUdpSocket* m_socket = nullptr;
	bool m_socketBound = false;

	RupFrame m_rupFrame;

	RupDataQueue m_rupDataQueue;

	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

	virtual void onTimer();

	void createAndBindSocket();
	void closeSocket();
	void onSocketReadyRead();

	virtual void clear();

public:
	DataChannel(int channel, DataSource::DataType dataType, const HostAddressPort& dataReceivingIP);
	virtual ~DataChannel();

	void addDataSource(DataSource* dataSource);

signals:

public slots:
};



class DataChannelThread : public SimpleThread
{
public:
	DataChannelThread();

	void addDataSource(DataSource* dataSource);
};


