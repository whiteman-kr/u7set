#pragma once

#include <QTimer>
#include <QUdpSocket>

#include "../lib/DataSource.h"
#include "../lib/DataProtocols.h"
#include "../lib/OrderedHash.h"
#include "../lib/SimpleThread.h"
#include "../lib/Queue.h"
#include "../lib/Signal.h"





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

	Rup::Frame m_rupFrame;

	RupDataQueue m_rupDataQueue;

	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

	virtual void onTimer();

	void createAndBindSocket();
	void closeSocket();

	virtual void clear();

private slots:
	void onSocketReadyRead();

public:
	DataChannel(int channel, DataSource::DataType dataType, const HostAddressPort& dataReceivingIP);
	virtual ~DataChannel();

	void addDataSource(DataSource* dataSource);
};


