#pragma once

#include "../AppSignalLib/SimpleAppSignalState.h"

namespace Gateway
{
	class AppSignalState
	{
	public:
		AppSignalState();
		AppSignalState(Hash appSignalIdHash);
		AppSignalState(const AppSignalState& appState);

		void updateState(const Proto::AppSignalState& protoState);
		const SimpleAppSignalState& getState() const;

		Hash hash() const;
		void setHash(Hash h);

		bool isWorkable() const;

	private:
		Hash m_hash = 0;
		std::atomic<int> m_writeStateIndex = { 0 };
		SimpleAppSignalState m_state[2];
	};

	using AppSignalStates = std::vector<AppSignalState>;
	using AppSignalStatesIterator = std::map<Hash, AppSignalState>::iterator;
}
