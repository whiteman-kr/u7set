#pragma once

#include "../VFrame30/TuningController.h"

class OutputController : public VFrame30::TuningController
{
public:
	OutputController() = delete;
	OutputController(ITuningSignalManager* signalManager, ITuningTcpClient* tcpClient, QWidget* parent = nullptr);
	OutputController(ITuningSignalManager* signalManager, std::vector<ITuningTcpClient*> tcpClients, QWidget* parent = nullptr);

};

