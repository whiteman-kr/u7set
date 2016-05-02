#pragma once

#include "../include/Signal.h"

struct AppSignalStateFlags
{
	quint32	valid : 1;
};


struct AppSignalState
{
public:
	qint64	serverTime = 0;
	qint64	pantTime = 0;

	AppSignalStateFlags flags;
	double value = 0;

	Signal* signal = nullptr;
	int index = 0;
};

