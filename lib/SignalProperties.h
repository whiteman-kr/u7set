#pragma once

#include "Signal.h"
#include "PropertyObject.h"


class SignalProperties : public PropertyObject
{
private:
	Signal m_signal;

	std::vector<Property*> m_propertiesDependentOnPrecision;

	void initProperties();

public:
	explicit SignalProperties(Signal& signal);

	Signal& signal() { return m_signal; }

	const std::vector<Property*>& propertiesDependentOnPrecision() { return m_propertiesDependentOnPrecision; }
};

