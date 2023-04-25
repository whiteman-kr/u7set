#pragma once

#include "../AppSignalLib/SimpleAppSignalState.h"

namespace Gateway
{
	class AppSignalState
	{
	public:
		AppSignalState(Hash appSignalIdHash);

		void updateState(const SimpleAppSignalState& state);
		const SimpleAppSignalState& getState() const;

		void switchStates();

	private:
		Hash m_hash = 0;
		std::atomic<int> m_writeStateIndex = { 0 };
		SimpleAppSignalState m_state[2];
	};

	using AppSignalStates = std::map<Hash, AppSignalState>;
	using AppSignalStatesIterator = std::map<Hash, AppSignalState>::iterator;
}
