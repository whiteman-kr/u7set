#pragma once

#include "../VFrame30/AppSignalController.h"

class AppSignalParam;
class AppSignalState;
class Comparator;

class InputController : public VFrame30::AppSignalController
{
	Q_OBJECT

public:
	InputController() = delete;
	explicit InputController(IAppSignalManager* appSignalManager, QObject* parent = nullptr);

};

