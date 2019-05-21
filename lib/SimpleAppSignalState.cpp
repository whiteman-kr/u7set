#include "SimpleAppSignalState.h"
#include "../Proto/serialization.pb.h"

// ---------------------------------------------------------------------------------------------------------
//
// SimpleAppSignalState struct implementation
//
// ---------------------------------------------------------------------------------------------------------

void SimpleAppSignalState::save(Proto::AppSignalState* protoState)
{
	if (protoState == nullptr)
	{
		assert(false);
		return;
	}

	assert(hash != 0);

	protoState->set_hash(hash);
	protoState->set_value(value);
	protoState->set_flags(flags.all);
	protoState->set_systemtime(time.system.timeStamp);
	protoState->set_localtime(time.local.timeStamp);
	protoState->set_planttime(time.plant.timeStamp);
	protoState->set_packetno(packetNo);
}

Hash SimpleAppSignalState::load(const Proto::AppSignalState& protoState)
{
	hash = protoState.hash();

	assert(hash != 0);

	value = protoState.value();
	flags.all = protoState.flags();

	time.system.timeStamp = protoState.systemtime();
	time.local.timeStamp = protoState.localtime();
	time.plant.timeStamp = protoState.planttime();
	packetNo = static_cast<quint16>(protoState.packetno());

	return hash;
}

void SimpleAppSignalState::print() const
{
	qDebug() << "state" << QDateTime::fromMSecsSinceEpoch(time.system.timeStamp).toString("dd.MM.yyyy HH:mm:ss.zzz") <<
				"validity =" << flags.valid <<
				"value =" << value <<
				(flags.autoPoint == 1 ? " auto" : "");
}

// ---------------------------------------------------------------------------------------------------------
//
// SimpleAppSignalStatesQueue class implementation
//
// ---------------------------------------------------------------------------------------------------------

SimpleAppSignalStatesQueue::SimpleAppSignalStatesQueue(int queueSize) :
	FastThreadSafeQueue<SimpleAppSignalState>(queueSize)
{
}

void SimpleAppSignalStatesQueue::pushAutoPoint(SimpleAppSignalState state, const QThread* thread)
{
	// state is a copy!
	//
	state.flags.autoPoint = 1;
	state.packetNo = 0;			// auto state

	push(state, thread);
}
