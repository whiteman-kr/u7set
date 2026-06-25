#pragma once
#include <span>


namespace GatewayClientLib
{
	template<typename SignalParamT, typename SignalStateT>
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
		virtual void addSignals(std::span<const SignalParamT> signals) = 0;

		// Invalidate all signal states.
		//
		virtual void invalidateSignalStates() = 0;

		// Set signal states by sources.
		//
		virtual void setStates(std::span<const SignalStateT> states) = 0;
	};
} // namespace GatewayClientLib