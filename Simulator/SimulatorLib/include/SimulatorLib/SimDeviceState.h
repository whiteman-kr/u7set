#pragma once

namespace Sim
{
	Q_NAMESPACE

	// DO NOT CHANGE the values of this enum, they are used in communication to VDU emulator.
	//
	enum class DeviceState
	{
		Off = 0,
		Start = 1,
		Fault = 2,
		Operate = 3
	};
	Q_ENUM_NS(DeviceState)

	enum class RuntimeMode
	{
		StartupMode,
		ConfigurationMode,
		RunSafeMode,
		RunMode,
		TuningMode,
		FaultedMode,
		PoweredOffMode,
	};

} // namespace Sim
