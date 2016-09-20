#include "../lib/AppSignalState.h"


void AppSignalState::setProtoAppSignalState(Proto::AppSignalState* protoState)
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

	protoState->set_systemtime(time.system);
	protoState->set_localtime(time.local);
	protoState->set_planttime(time.plant);
}


Hash AppSignalState::getProtoAppSignalState(const Proto::AppSignalState* protoState)
{
	if (protoState == nullptr)
	{
		assert(false);
		return 0 ;
	}

	assert(hash != 0);

	hash = protoState->hash();
	value = protoState->value();
	flags.all = protoState->flags();

	time.system = protoState->systemtime();
	time.local = protoState->localtime();
	time.plant = protoState->planttime();

	return hash;
}
