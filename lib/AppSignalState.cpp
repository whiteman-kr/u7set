#include "../lib/AppSignalState.h"


QDateTime Times::systemToDateTime() const
{
	return QDateTime::fromMSecsSinceEpoch(system);
}

QDateTime Times::localToDateTime() const
{
	return QDateTime::fromMSecsSinceEpoch(local);
}

QDateTime Times::plantToDateTime() const
{
	return QDateTime::fromMSecsSinceEpoch(plant);
}

bool AppSignalState::isValid() const
{
	return flags.valid;
}

bool AppSignalState::isOverflow() const
{
	return flags.overflow;
}

bool AppSignalState::isUnderflow() const
{
	return flags.underflow;
}

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

	hash = protoState->hash();

	assert(hash != 0);

	value = protoState->value();
	flags.all = protoState->flags();

	time.system = protoState->systemtime();
	time.local = protoState->localtime();
	time.plant = protoState->planttime();

	return hash;
}
