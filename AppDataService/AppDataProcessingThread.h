#pragma once

#include "AppDataServiceTypes.h"
#include "../lib/SimpleThread.h"
#include "../lib/DataChannel.h"
#include "AppSignalStateEx.h"


class AppDataProcessingWorker : public SimpleThreadWorker
{
private:
	int m_number = 0;
	RupDataQueue& m_rupDataQueue;
	const SourceParseInfoMap& m_sourceParseInfoMap;
	AppSignalStates& m_signalStates;
	AppSignalStatesQueue& m_signalStatesQueue;

	RupData m_rupData;					// parsing buffer

	// parsing statistics
	//
	quint64 m_parsedRupDataCount = 0;
	quint64 m_notFoundIPCount = 0;
	quint64 m_valueParsingErrorCount = 0;
	quint64 m_validityParsingErrorCount = 0;
	quint64 m_badSignalStateIndexCount = 0;

	//

	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

	void parseRupData();
	bool getDoubleValue(const SignalParseInfo& parseInfo, double& value);
	bool getValidity(const SignalParseInfo& parseInfo, quint32& validity);

public:
	AppDataProcessingWorker(int number,
							RupDataQueue& rupDataQueue,
							const SourceParseInfoMap& sourceParseInfoMap,
							AppSignalStates& signalStates,
							AppSignalStatesQueue& signalStatesQueue);

public slots:
	void slot_rupDataQueueIsNotEmpty();
};


class AppDataProcessingThread : public SimpleThread
{
public:
 AppDataProcessingThread(int number, RupDataQueue& rupDataQueue,
							const SourceParseInfoMap& sourceParseInfoMap,
							AppSignalStates& signalStates,
							AppSignalStatesQueue& signalStatesQueue);
};


class AppDataProcessingThreadsPool : public QList<AppDataProcessingThread*>
{
public:
	void createProcessingThreads(int poolSize, RupDataQueue& rupDataQueue,
				const SourceParseInfoMap& sourceParseInfoMap,
				AppSignalStates& signalStates,
				AppSignalStatesQueue& signalStatesQueue);

	void startProcessingThreads();

	void stopAndClearProcessingThreads();
};

