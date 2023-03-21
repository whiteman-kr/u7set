#pragma once

#include "../VFrame30/TuningController.h"

class OutputController : public VFrame30::TuningController
{
public:
	OutputController() = delete;
	OutputController(ITuningSignalManager* signalManager, ITuningConnection* tuningConnection, QWidget* parent = nullptr);

};

