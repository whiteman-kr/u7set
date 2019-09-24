#pragma once

#include "Hash.h"
#include "Queue.h"
#include "Times.h"
#include "AppSignalStateFlags.h"

namespace Proto
{
	class AppSignalState;
}

struct SimpleAppSignalState
{
	// light version of AppSignalState to use in queues and other AppDataService data structs
	//
	Hash hash = 0;					// == calcHash(AppSignalID)
	Times time;
	AppSignalStateFlags flags;
	double value = 0;
	quint16 packetNo = 0;

	bool isValid() const { return flags.valid == 1; }

	void save(Proto::AppSignalState* protoState);
	Hash load(const Proto::AppSignalState& protoState);

	void print() const;
};

class SimpleAppSignalStatesQueue : public FastThreadSafeQueue<SimpleAppSignalState>
{
public:
	SimpleAppSignalStatesQueue(int queueSize);
};

typedef std::shared_ptr<SimpleAppSignalStatesQueue> SimpleAppSignalStatesQueueShared;


struct SimpleAppSignalStateArchiveFlag
{
	SimpleAppSignalState state;
	bool sendStateToArchive = false;
};

class SimpleAppSignalStatesArchiveFlagQueue : public FastThreadSafeQueue<SimpleAppSignalStateArchiveFlag>
{
public:
	SimpleAppSignalStatesArchiveFlagQueue(int queueSize);

	void push(const SimpleAppSignalState& state, bool sendStateToArchive, const QThread* thread);
	void pushAutoPoint(const SimpleAppSignalState& state, bool sendStateToArchive, const QThread* thread);
};







