#pragma once

#include "Signal.h"
#include "PropertyObject.h"


class SignalProperties : public PropertyObject
{
private:
	Signal& m_signal;

	void initProperties();

public:
	explicit SignalProperties(Signal& signal);
};

