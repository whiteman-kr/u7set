#pragma once
#include <span>

namespace ClientLib
{
	class IAppSignalUpdater
	{
	public:
		virtual ~IAppSignalUpdater() = default;

		// Reset all signal params and states.
		//
		virtual void reset() = 0;

		// This function must notify (emit signal?) that signal params where updated.
		// This signal is out of scope of this interface, it is up to implementation which signal to emit and how to use it.
		//
		virtual void notifySignalParamsUpdated() = 0;

		// Set signal params.
		//
		virtual void addSignal(const AppSignalParam& appSignal, const QString& appDataServiceId) = 0;
		virtual void addSignals(std::span<const AppSignalParam> appSignals, const QString& appDataServiceId) = 0;

		// Invalidate all signal states by source sourceThreadId.
		//
		virtual void invalidateSignalStates(Qt::HANDLE sourceThreadId) = 0;

		// Set signal states by sources.
		//
		virtual void setState(const QString& appSignalId, const AppSignalState& state, Hash dataServerHash, Qt::HANDLE sourceThreadId) = 0;
		virtual void setState(Hash signalHash, const AppSignalState& state, Hash dataServerHash, Qt::HANDLE sourceThreadId) = 0;
		virtual void setState(std::span<const AppSignalState> states, Hash dataServerHash, Qt::HANDLE sourceThreadId) = 0;
	};

}
