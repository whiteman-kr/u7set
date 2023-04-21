#include "SimTuningConnection.h"


bool SimTuningConnection::writeTuningSignal(const QString& /*appSignalId*/, const TuningValue& /*value*/)
{
	assert(false);
	return false;
}

bool SimTuningConnection::writeTuningSignal(const QString& /*appSignalId*/, QVariant /*value*/)
{
	assert(false);
	return false;
}

void SimTuningConnection::applyTuningSignals()
{
	assert(false);
	return;
}
