#include "SimTuningTcpClient.h"

SimTuningTcpClient::SimTuningTcpClient()
{
}

bool SimTuningTcpClient::writeTuningSignal(QString /*appSignalId*/, TuningValue /*value*/)
{
	assert(false);
	return false;
}
