#include "TuningConnectionStub.h"

bool TuningConnectionStub::writeTuningSignal(const QString& /*appSignalId*/, const TuningValue& /*value*/)
{
	assert(false);
	return false;
}

bool TuningConnectionStub::writeTuningSignal(const QString& /*appSignalId*/, QVariant /*value*/)
{
	assert(false);
	return false;
}

void TuningConnectionStub::applyTuningSignals()
{
	assert(false);
	return;
}
