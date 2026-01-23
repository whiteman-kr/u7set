#pragma once

#include <AdsGatewayLib/AdsGwProtocol.hpp>

#include <span>


namespace AdsGatewayLib
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
		virtual void addSignals(std::span<const AdsGatewayLib::GwAppSignalParam> signals) = 0;

		// Invalidate all signal states.
		//
		virtual void invalidateSignalStates() = 0;

		// Set signal states by sources.
		//
		virtual void setStates(std::span<const AdsGatewayLib::GwAppSignalState> states) = 0;
	};
} // namespace AdsGatewayLib