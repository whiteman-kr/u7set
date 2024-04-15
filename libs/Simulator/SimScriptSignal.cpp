#include "SimScriptSignal.h"

namespace Sim
{

	ScriptSignal::ScriptSignal(const AppSignal& s) :
		m_signal(s)
	{
	}

	bool ScriptSignal::isNull() const
	{
		return m_signal.appSignalID().isEmpty();
	}

	void ScriptSignal::setSignal(const AppSignal& s)
	{
		m_signal = s;
	}
}
