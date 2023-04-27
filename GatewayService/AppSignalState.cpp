#include "AppSignalState.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	//	Gateway::AppSignalState class implementation
	//
	// ---------------------------------------------------------------------------------

	AppSignalState::AppSignalState()
	{
	}

	AppSignalState::AppSignalState(Hash appSignalIdHash) :
		m_hash(appSignalIdHash)
	{
	}

	AppSignalState::AppSignalState(const AppSignalState& appState)
	{
		m_hash = appState.hash();
	}

	void AppSignalState::updateState(const Proto::AppSignalState& protoState)
	{
		if (m_hash != protoState.hash())
		{
			Q_ASSERT(false);
			return;
		}

		m_state[m_writeStateIndex].load(protoState);
		m_writeStateIndex.fetch_xor(1);
	}

	const SimpleAppSignalState& AppSignalState::getState() const
	{
		return m_state[m_writeStateIndex ^ 1];
	}

	Hash AppSignalState::hash() const
	{
		return m_hash;
	}

	void AppSignalState::setHash(Hash h)
	{
		m_hash = h;
	}

	int AppSignalState::listIndex() const
	{
		return m_listIndex;
	}

	void AppSignalState::setListIndex(int index)
	{
		m_listIndex = index;
	}


	bool AppSignalState::isWorkable() const
	{
		return m_hash != 0;
	}
}
