#include "SimTuningTcpClient.h"


SimTuningTcpClient::SimTuningTcpClient()
{
}

bool SimTuningTcpClient::hasTuningSignal(QString /*appSignalId*/) const
{
	assert(false);
	return false;
}

bool SimTuningTcpClient::writeTuningSignal(QString /*appSignalId*/, TuningValue /*value*/)
{
	assert(false);
	return false;
}

void SimTuningTcpClient::applyTuningSignals()
{
	assert(false);
	return;
}
