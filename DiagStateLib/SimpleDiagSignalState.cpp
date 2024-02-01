#ifndef DIAG_STATE_LIB_DOMAIN
#error Do not include this file in the project! Link DiagStateLib instead.
#endif

#include "SimpleDiagSignalState.h"
#include "../UtilsLib/WUtils.h"

// ---------------------------------------------------------------------------------------------------------
//
// SimpleAppSignalState struct implementation
//
// ---------------------------------------------------------------------------------------------------------

//SimpleAppSignalState::operator AppSignalState() const
//{
//	AppSignalState state;

//	copyTo(state);

//	return state;
//}

//void SimpleAppSignalState::save(Proto::AppSignalState* protoState) const
//{
//	TEST_PTR_RETURN(protoState);

//	if (hash == 0)
//	{
//		protoState->Clear();
//		return;
//	}

//	protoState->set_hash(hash);
//	protoState->set_value(value);
//	protoState->set_flags(flags.all);
//	protoState->set_systemtime(time.system.timeStamp);
//	protoState->set_localtime(time.local.timeStamp);
//	protoState->set_planttime(time.plant.timeStamp);
//	protoState->set_packetno(packetNo);
//}

//Hash SimpleAppSignalState::load(const Proto::AppSignalState& protoState)
//{
//	hash = protoState.hash();

//	value = protoState.value();
//	flags.all = protoState.flags();

//	time.system.timeStamp = protoState.systemtime();
//	time.local.timeStamp = protoState.localtime();
//	time.plant.timeStamp = protoState.planttime();
//	packetNo = static_cast<quint16>(protoState.packetno());

//	return hash;
//}

//void SimpleAppSignalState::print() const
//{
//	qDebug() << "state" << QDateTime::fromMSecsSinceEpoch(time.system.timeStamp).toString("dd.MM.yyyy HH:mm:ss.zzz") <<
//				"validity =" << flags.valid <<
//				"value =" << value <<
//				(flags.autoPoint == 1 ? " auto" : "");
//}

// ---------------------------------------------------------------------------------------------------------
//
// SimpleDiagSignalStatesQueue class implementation
//
// ---------------------------------------------------------------------------------------------------------

SimpleDiagSignalStatesQueue::SimpleDiagSignalStatesQueue(int queueSize) :
	FastThreadSafeQueue<SimpleDiagSignalState>(queueSize)
{
}

SimpleDiagSignalStatesQueue::~SimpleDiagSignalStatesQueue()
{
}

void SimpleDiagSignalStatesQueue::afterPush()
{
	m_afterPushCtr++;

	if (m_afterPushCtr > 50)
	{
		m_afterPushCtr = 0;

		emit queueNotEmpty();
	}
}

