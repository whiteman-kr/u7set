#pragma once
#include "ISignalUpdater.hpp"

#include <AdsGatewayLib/GwHash.hpp>

#include <mutex>
#include <optional>
#include <unordered_map>


namespace AdsGatewayLib
{
	class SignalManager : public ISignalUpdater
	{
	public:
		// ISignalUpdater implementation
		//
		void reset() override;
		void addSignals(std::span<const AdsGatewayLib::GwAppSignalParam> signals) override;
		void invalidateSignalStates() override;
		void setStates(std::span<const AdsGatewayLib::GwAppSignalState> states) override;

		// --
		//
		std::optional<GwAppSignalParam> getSignalParam(std::string_view signalId) const;
		std::optional<GwAppSignalParam> getSignalParam(Radiy::Hash signalIdHash) const;

		std::optional<GwAppSignalState> getSignalState(std::string_view signalId) const;
		std::optional<GwAppSignalState> getSignalState(Radiy::Hash signalIdHash) const;

	private:
		mutable std::mutex m_paramsMutex;
		std::unordered_map<Radiy::Hash, GwAppSignalParam> m_params;

		mutable std::mutex m_statesMutex;
		std::unordered_map<Radiy::Hash, GwAppSignalState> m_states;
	};
} // namespace AdsGatewayLib