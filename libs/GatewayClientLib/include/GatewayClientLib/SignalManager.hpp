#pragma once
#include "GwHash.hpp"
#include "ISignalUpdater.hpp"

#include <mutex>
#include <optional>
#include <unordered_map>


namespace GatewayClientLib
{
	template<typename SignalParamT, typename SignalStateT>
	class SignalManager : public ISignalUpdater<SignalParamT, SignalStateT>
	{
	public:
		void reset() override;

		void addSignals(std::span<const SignalParamT> signals) override;
		void invalidateSignalStates() override;
		void setStates(std::span<const SignalStateT> states) override;

		// --
		//
		std::optional<SignalParamT> getSignalParam(std::string_view signalId) const;
		std::optional<SignalParamT> getSignalParam(Radiy::Hash signalIdHash) const;

		std::optional<SignalStateT> getSignalState(std::string_view signalId) const;
		std::optional<SignalStateT> getSignalState(Radiy::Hash signalIdHash) const;

	private:
		mutable std::mutex m_paramsMutex;
		std::unordered_map<Radiy::Hash, SignalParamT> m_params;

		mutable std::mutex m_statesMutex;
		std::unordered_map<Radiy::Hash, SignalStateT> m_states;
	};

	template<typename SignalParamT, typename SignalStateT>
	void SignalManager<SignalParamT, SignalStateT>::reset()
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

	template<typename SignalParamT, typename SignalStateT>
	void SignalManager<SignalParamT, SignalStateT>::addSignals(std::span<const SignalParamT> signals)
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

	template<typename SignalParamT, typename SignalStateT>
	void SignalManager<SignalParamT, SignalStateT>::invalidateSignalStates()
	{
		std::lock_guard lock{m_statesMutex};
		m_states.clear();
	}

	template<typename SignalParamT, typename SignalStateT>
	void SignalManager<SignalParamT, SignalStateT>::setStates(std::span<const SignalStateT> states)
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

	template<typename SignalParamT, typename SignalStateT>
	std::optional<SignalParamT> SignalManager<SignalParamT, SignalStateT>::getSignalParam(std::string_view signalId) const
	{
		Radiy::Hash signalIdHash = Radiy::calcHash(signalId);
		return getSignalParam(signalIdHash);
	}

	template<typename SignalParamT, typename SignalStateT>
	std::optional<SignalParamT> SignalManager<SignalParamT, SignalStateT>::getSignalParam(Radiy::Hash signalId) const
	{
		std::lock_guard lock{m_paramsMutex};

		auto it = m_params.find(signalId);
		if (it != m_params.end())
		{
			return it->second;
		}

		return std::nullopt;
	}

	template<typename SignalParamT, typename SignalStateT>
	std::optional<SignalStateT> SignalManager<SignalParamT, SignalStateT>::getSignalState(std::string_view signalId) const
	{
		Radiy::Hash signalIdHash = Radiy::calcHash(signalId);
		return getSignalState(signalIdHash);
	}

	template<typename SignalParamT, typename SignalStateT>
	std::optional<SignalStateT> SignalManager<SignalParamT, SignalStateT>::getSignalState(Radiy::Hash signalId) const
	{
		std::lock_guard lock{m_statesMutex};

		auto it = m_states.find(signalId);
		if (it != m_states.end())
		{
			return it->second;
		}

		return std::nullopt;
	}
} // namespace GatewayClientLib