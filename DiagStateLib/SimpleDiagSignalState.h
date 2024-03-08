#pragma once

#include "../CommonLib/Hash.h"
#include "../CommonLib/Times.h"
#include "../UtilsLib/Queue.h"
#include "DiagSignalStateFlags.h"

struct SimpleDiagSignalState
{
	// light version of AppSignalState to use in queues and other AppDataService data structs
	//
	Hash hash = 0;					// == calcHash(AppSignalID)
	Times time{};
	DiagSignalStateFlags flags{};
	double value = 0;

	//quint16 packetNo = 0;
	//operator ::AppSignalState() const;

//	inline void copyTo(::AppSignalState& state) const
//	{
//		state.m_hash = hash;
//		state.m_time = time;
//		state.m_flags = flags;
//		state.m_value = value;
//	}

	bool isValid() const { return flags.valid == 1; }

//	void save(Proto::AppSignalState* protoState) const;
//	Hash load(const Proto::AppSignalState& protoState);

//	void print() const;

	qint64 plantTime() const { return time.plant.timeStamp; }
	qint64 systemTime() const { return time.system.timeStamp; }
	qint64 localTime() const { return time.local.timeStamp; }
};

//template <typename SIGNAL_STATE>
//class SignalStatesQueue : public QObject, public FastThreadSafeQueue<SIGNAL_STATE>
//{
//	Q_OBJECT

//public:
//	SignalStatesQueue(int queueSize) : FastThreadSafeQueue<SIGNAL_STATE>(queueSize) {}
//	virtual ~SignalStatesQueue() {}

//signals:
//	void queueNotEmpty();

//private:
//	virtual void afterPush() override
//	{
//		m_afterPushCtr++;

//		if (m_afterPushCtr > 50)
//		{
//			m_afterPushCtr = 0;

//			emit queueNotEmpty();
//		}
//	}

//private:
//	int m_afterPushCtr = 0;
//};


