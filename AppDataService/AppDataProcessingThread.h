#pragma once

#include "AppDataServiceTypes.h"
#include "../include/SimpleThread.h"
#include "../include/DataChannel.h"


class AppDataProcessingWorker : public SimpleThreadWorker
{
private:
	RupDataQueue& m_rupDataQueue;
	const SourceParseInfoMap& m_sourceParseInfoMap;
	AppSignalStates& m_signalStates;

public:
	AppDataProcessingWorker(RupDataQueue& rupDataQueue,
							const SourceParseInfoMap& sourceParseInfoMap,
							AppSignalStates& signalStates);
};


class AppDataProcessingThread : public SimpleThread
{
public:
 AppDataProcessingThread(RupDataQueue& rupDataQueue,
							const SourceParseInfoMap& sourceParseInfoMap,
							AppSignalStates& signalStates);
};


class AppDataProcessingThreadsPool : public QList<AppDataProcessingThread*>
{
public:
	void start(int poolSize, RupDataQueue& rupDataQueue,
				const SourceParseInfoMap& sourceParseInfoMap,
				AppSignalStates& signalStates);

	void stopAndClear();
};

