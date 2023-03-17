#include "SimTuningConnection.h"


SimTuningConnection::SimTuningConnection()
{
}

bool SimTuningConnection::hasTuningSignal(QString /*appSignalId*/) const
{
	assert(false);
	return false;
}

bool SimTuningConnection::writeTuningSignal(QString /*appSignalId*/, TuningValue /*value*/)
{
	assert(false);
	return false;
}

void SimTuningConnection::applyTuningSignals()
{
	assert(false);
	return;
}
