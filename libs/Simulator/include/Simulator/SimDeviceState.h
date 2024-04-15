#pragma once

namespace Sim
{
	Q_NAMESPACE

	enum class DeviceState
	{
		Off,
		Start,
		Fault,
		Operate
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
