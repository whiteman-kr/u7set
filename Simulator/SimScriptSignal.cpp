#include "SimScriptSignal.h"

namespace Sim
{

	ScriptSignal::ScriptSignal(const Signal& s) :
		m_signal(s)
	{
	}

	bool ScriptSignal::isNull() const
	{
		return m_signal.appSignalID().isEmpty();
	}

	void ScriptSignal::setSignal(const Signal& s)
	{
		m_signal = s;
	}

}
