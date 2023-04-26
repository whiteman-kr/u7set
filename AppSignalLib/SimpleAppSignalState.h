#pragma once

#include "../CommonLib/Hash.h"
#include "../CommonLib/Times.h"
#include "../UtilsLib/Queue.h"
#include "../Proto/serialization.pb.h"
#include "AppSignalParam.h"

struct SimpleAppSignalState
{
	// light version of AppSignalState to use in queues and other AppDataService data structs
	//
	Hash hash = 0;					// == calcHash(AppSignalID)
	Times time;
	AppSignalStateFlags flags;
	double value = 0;
	quint16 packetNo = 0;

	operator AppSignalState() const;

	inline void copyTo(AppSignalState& state) const
	{
		state.m_hash = hash;
		state.m_time = time;
		state.m_flags = flags;
		state.m_value = value;
	}

	bool isValid() const { return flags.valid == 1; }

	void save(Proto::AppSignalState* protoState);
	Hash load(const Proto::AppSignalState& protoState);

	void print() const;
};

class SimpleAppSignalStatesQueue : public QObject, public FastThreadSafeQueue<SimpleAppSignalState>
{
	Q_OBJECT

public:
	enum class ReceiveMode
	{
		Continue = 0,					// no change mode, continue in previously set mode

		AllSignals = 1,					// receive all signals, default mode
		SelectedSignals = 2,			// receive only selected signals
	};

public:
	SimpleAppSignalStatesQueue(int queueSize);
	virtual ~SimpleAppSignalStatesQueue();

	virtual void push(const SimpleAppSignalState& item, const QThread* thread, int* curSize = nullptr, int* curMaxSize = nullptr) override;

	void setReceiveMode(ReceiveMode mode, std::set<Hash>& selectedHashes);

signals:
	void queueNotEmpty();

private:
	virtual void afterPush() override;

private:
	int m_afterPushCtr = 0;

	SimpleMutex m_receiveModeMutex;
	ReceiveMode m_receiveMode = ReceiveMode::AllSignals;
	std::set<Hash> m_selectedHashes;
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







