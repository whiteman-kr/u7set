#pragma once

#include "../CommonLib/Hash.h"
#include "../CommonLib/Times.h"
#include "../UtilsLib/Queue.h"
#include "AppSignalState.h"

namespace Proto
{
	class AppSignalState;
}

namespace Network
{
	class GatewayAppSignalState;
}

struct SimpleAppSignalState
{
	// light version of AppSignalState to use in queues and other AppDataService data structs
	//
	Hash hash = 0;					// == calcHash(AppSignalID)
	Times time{};
	AppSignalStateFlags flags{};
	double value = 0;
	quint16 packetNo = 0;

	operator ::AppSignalState() const;

	inline void copyTo(::AppSignalState& state) const
	{
		state.m_hash = hash;
		state.m_time = time;
		state.m_flags = flags;
		state.m_value = value;
	}

	bool isValid() const { return flags.valid == 1; }

	void save(Proto::AppSignalState* protoState) const;
	Hash load(const Proto::AppSignalState& protoState);

	void print() const;

	qint64 plantTime() const { return time.plant.timeStamp; }
	qint64 systemTime() const { return time.system.timeStamp; }
	qint64 localTime() const { return time.local.timeStamp; }
};

class SimpleAppSignalStatesQueue : public QObject, public FastThreadSafeQueue<SimpleAppSignalState>
{
	Q_OBJECT

public:
	SimpleAppSignalStatesQueue(int queueSize);
	virtual ~SimpleAppSignalStatesQueue();

signals:
	void queueNotEmpty();

private:
	virtual void afterPush() override;

private:
	int m_afterPushCtr = 0;
};

using SimpleAppSignalStatesQueueShared = std::shared_ptr<SimpleAppSignalStatesQueue>;

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

struct GatewayAppSignalState
{
	SimpleAppSignalState prevState;
	SimpleAppSignalState curState;

	void saveToProto(::Network::GatewayAppSignalState* proto) const;
	void loadFromProto(const ::Network::GatewayAppSignalState& proto);
};

struct GatewayAppSignalStateQueueMask
{
	quint32 gatewayQueueMask = 0;
	GatewayAppSignalState gwState;
};

using GatewayAppSignalStatesQueue = FastThreadSafeQueue<GatewayAppSignalStateQueueMask>;
using GatewayAppSignalStatesQueueShared = std::shared_ptr<GatewayAppSignalStatesQueue>;


