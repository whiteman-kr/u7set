#include <AdsGatewayLib/SignalManager.hpp>

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

	std::optional<GwAppSignalParam> SignalManager::getSignalParam(std::string_view signalId) const
	{
		Radiy::Hash signalIdHash = Radiy::calcHash(signalId);
		return getSignalParam(signalIdHash);
	}

	std::optional<GwAppSignalParam> SignalManager::getSignalParam(Radiy::Hash signalId) const
	{
		std::lock_guard lock{m_paramsMutex};

		auto it = m_params.find(signalId);
		if (it != m_params.end())
		{
			return it->second;
		}

		return std::nullopt;
	}

	std::optional<GwAppSignalState> SignalManager::getSignalState(std::string_view signalId) const
	{
		Radiy::Hash signalIdHash = Radiy::calcHash(signalId);
		return getSignalState(signalIdHash);
	}

	std::optional<GwAppSignalState> SignalManager::getSignalState(Radiy::Hash signalId) const
	{
		std::lock_guard lock{m_statesMutex};

		auto it = m_states.find(signalId);
		if (it != m_states.end())
		{
			return it->second;
		}

		return std::nullopt;
	}
} // namespace AdsGatewayLib