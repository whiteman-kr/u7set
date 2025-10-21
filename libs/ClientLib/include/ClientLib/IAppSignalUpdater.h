#pragma once
#include <span>

namespace ClientLib
{
	class IAppSignalUpdater
	{
	public:
		virtual ~IAppSignalUpdater() = default;

	public:
		using SourceIdType = uintptr_t;

		// Reset all signal params and states.
		//
		virtual void reset() = 0;

		// This function must notify (emit signal?) that signal params where updated.
		// This signal is out of scope of this interface, it is up to implementation which signal to emit and how to use it.
		//
		virtual void notifySignalParamsUpdated() = 0;

		// Set signal params.
		//
		virtual void addSignals(std::span<const AppSignalParam> appSignals, const QString& appDataServiceId) = 0;

		// Invalidate all signal states by source sourceThreadId.
		//
		virtual void invalidateSignalStates(SourceIdType sourceThreadId) = 0;

		// Set signal states by sources.
		//
		virtual void setStates(std::span<const AppSignalState> states, Hash dataServerHash, SourceIdType sourceThreadId) = 0;
	};

} // namespace ClientLib
