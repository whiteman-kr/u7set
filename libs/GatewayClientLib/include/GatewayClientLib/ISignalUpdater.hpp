#pragma once

#include "AdsGwProtocol.hpp"

#include <span>


namespace GatewayClientLib
{
	class ISignalUpdater
	{
	public:
		virtual ~ISignalUpdater() = default;

	public:
		// Reset all signal params and states.
		//
		virtual void reset() = 0;

		// Set signal params.
		//
		virtual void addSignals(std::span<const GatewayClientLib::GwAppSignalParam> signals) = 0;

		// Invalidate all signal states.
		//
		virtual void invalidateSignalStates() = 0;

		// Set signal states by sources.
		//
		virtual void setStates(std::span<const GatewayClientLib::GwAppSignalState> states) = 0;
	};
} // namespace GatewayClientLib