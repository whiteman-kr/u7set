#pragma once
#include "SimDeviceState.h"
#include <chrono>

namespace Sim
{
	struct ControlData;

	enum class SimControlState
	{
		Stop,
		Run,
		Pause
	};

	struct ControlStatus
	{
		ControlStatus() = default;
		explicit ControlStatus(const ControlData& cd);

		std::chrono::microseconds m_startTime{0};   // When simulation was started, it's computer time
		std::chrono::microseconds m_currentTime{0}; // Current time in simulation

		std::chrono::microseconds m_duration{0};
		SimControlState m_state = SimControlState::Stop;

		struct LmMode
		{
			QString lmEquipmentId;
			Sim::DeviceState deviceState;
		};

		std::vector<Sim::ControlStatus::LmMode> m_lmDeviceModes;
	};
} // namespace Sim

Q_DECLARE_METATYPE(Sim::SimControlState);
Q_DECLARE_METATYPE(Sim::ControlStatus);