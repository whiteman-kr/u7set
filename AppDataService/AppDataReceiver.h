#pragma once

#include "../lib/DataChannel.h"
#include "AppSignalStateEx.h"
#include "AppDataServiceTypes.h"
#include "AppDataProcessingThread.h"

//
// AppDataChannel receive RUP datagramms
// and send
//

class AppDataReceiver : public SimpleThreadWorker
{
public:
	AppDataReceiver(const HostAddressPort& dataReceivingIP);
	virtual ~AppDataReceiver();

	void prepare(AppSignals& appSignals, AppSignalStates* signalStates);
	void addDataSource(AppDataSource* appDataSource);

private:
	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

	void createAndBindSocket();
	void closeSocket();
	
	void checkDataSourcesDataReceiving();
	void invalidateDataSourceSignals(quint32 dataSourceIP, qint64 currentTime);

	void clear();

private slots:	
	void onTimer1s();
	void onSocketReadyRead();

private:
	// AppDataChannel settings
	//
	HostAddressPort m_dataReceivingIP;
	HashedVector<quint32, AppDataSource*> m_appDataSources;			// allocated and freed in AppDataService
	HashedVector<quint32, quint32> m_unknownDataSources;
	int m_autoArchivingGroupsCount = 0;

	// AppDataChannel sockets
	//
	const int PACKET_TIMEOUT = 1000;							// 1000 ms == 1 second
	QUdpSocket* m_socket = nullptr;
	bool m_socketBound = false;
	Rup::Frame m_rupFrame;
	RupDataQueue m_rupDataQueue;
	QTimer m_timer1s;

	int m_receivedFramesCount = 0;

	// AppDataChannel parsing
	//
	SourceParseInfoMap m_sourceParseInfoMap;		// source ip => QVector<SignalParseInfo> map
	AppSignalStates* m_signalStates = nullptr;		// allocated and freed in AppDataService
	AppDataProcessingThreadsPool m_processingThreadsPool;
	AppSignalStatesQueue& m_signalStatesQueue;
};


// This thread need to read UDP datagramms (inside DataChannel) and push it to the m_rupDataQueue
//
class AppDataReceiverlThread : public SimpleThread
{
public:
	AppDataReceiverlThread(	const HostAddressPort& dataRecievingIP);

	void prepare(AppSignals &appSignals, AppSignalStates *signalStates);
	void addDataSource(AppDataSource* appDataSource);
};

