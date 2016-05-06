#pragma once

#include "../include/DataChannel.h"
#include "AppSignalState.h"
#include "AppDataServiceTypes.h"
#include "AppDataProcessingThread.h"


class AppDataChannel : public DataChannel
{
private:
	SourceParseInfoMap m_sourceParseInfoMap;		// source ip => QVector<SignalParseInfo> map

	AppSignalStates* m_signalStates = nullptr;		// allocated and freed in AppDataService

	AppDataProcessingThreadsPool m_processingThreadsPool;

	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

protected:
	virtual void clear() override;

public:
	AppDataChannel(int channel, const HostAddressPort& dataReceivingIP);
	virtual ~AppDataChannel();

	void prepare(AppSignals& appSignals, AppSignalStates* signalStates);
};


// This thread need to read UDP datagramms (inside DataChannel) and push it to the m_rupDataQueue
//
class AppDataChannelThread : public SimpleThread
{
private:
	AppDataChannel* m_appDataChannel = nullptr;

public:
	AppDataChannelThread(int channel, const HostAddressPort& dataRecievingIP);

	void prepare(AppSignals &appSignals, AppSignalStates *signalStates);
	void addDataSource(DataSource* dataSource);
};

