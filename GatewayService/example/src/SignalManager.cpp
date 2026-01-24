#include "SignalManager.hpp"

#include <cassert>

namespace AdsGatewayLib
{
	void SignalManager::reset()
	{
		{
			std::lock_guard lockParams{m_paramsMutex};
			m_params.clear();
		}

		{
			std::lock_guard lockStates{m_statesMutex};
			m_states.clear();
		}
	}

	void SignalManager::addSignals(std::span<const AdsGatewayLib::GwAppSignalParam> signals)
	{
		size_t paramCount = 0;

		{
			std::lock_guard lock{m_paramsMutex};
			for (const auto& signal : signals)
			{
				assert(signal.hash == Radiy::calcHash(signal.appSignalId));
				m_params[signal.hash] = signal;
			}

			paramCount = m_params.size();
		}

		{
			std::lock_guard lock{m_statesMutex};
			if (m_states.size() < paramCount)
			{
				m_states.reserve(paramCount);
			}
		}
	}

	void SignalManager::invalidateSignalStates()
	{
		std::lock_guard lock{m_statesMutex};
		m_states.clear();
	}

	void SignalManager::setStates(std::span<const AdsGatewayLib::GwAppSignalState> states)
	{
		std::lock_guard lock{m_statesMutex};

		for (const auto& state : states)
		{
			if (state.hash != Radiy::UNDEFINED_HASH)
			{
				m_states[state.hash] = state;
			}
		}
	}
} // namespace AdsGatewayLib