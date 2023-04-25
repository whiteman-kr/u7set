#include "AppSignalState.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	//	Gateway::AppSignalState class implementation
	//
	// ---------------------------------------------------------------------------------

	void AppSignalState::updateState(const SimpleAppSignalState& state)
	{
		m_state[m_writeStateIndex] = state;
		m_writeStateIndex.fetch_xor(1);
	}

	const SimpleAppSignalState& AppSignalState::getState() const
	{
		return m_state[m_writeStateIndex ^ 1];
	}

}
