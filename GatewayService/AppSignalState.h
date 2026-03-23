#pragma once

#include "../AppSignalLib/SimpleAppSignalState.h"

namespace Gateway
{
	class AppSignalState
	{
	public:
		AppSignalState();
		AppSignalState(Hash appSignalIdHash, bool requestEvents);
		AppSignalState(const AppSignalState& appState);

		void updateState(const Proto::AppSignalState& protoState);
		const SimpleAppSignalState& getState() const;

		void invalidate();

		Hash hash() const;
		void setHash(Hash h);

		int listIndex() const;
		void setListIndex(int index);

		bool isWorkable() const;

		bool requestEvents() const;

	private:
		Hash m_hash = 0;
		int m_listIndex = 0;
		bool m_requestEvents = false;
		std::atomic<int> m_writeStateIndex = { 0 };
		SimpleAppSignalState m_state[2];
	};

	using AppSignalStates = std::vector<AppSignalState>;
	using AppSignalStatesIterator = std::map<Hash, AppSignalState>::iterator;
}
