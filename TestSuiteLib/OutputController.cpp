#include "OutputController.h"

OutputController::OutputController(ITuningSignalManager* signalManager, ITuningTcpClient* tcpClient, QWidget* parent):
	VFrame30::TuningController(signalManager, tcpClient, parent)
{

}

OutputController::OutputController(ITuningSignalManager* signalManager, std::vector<ITuningTcpClient*> tcpClients, QWidget* parent):
	VFrame30::TuningController(signalManager, tcpClients, parent)
{

}
