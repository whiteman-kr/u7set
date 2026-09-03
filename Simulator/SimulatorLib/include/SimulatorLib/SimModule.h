#pragma once

#include "SimDeviceState.h"

#include <HardwareLib/LogicModulesInfo.h>
#include <SimulatorLib/SimModuleJumpers.h>

namespace Sim
{
	class BaseModuleImpl;

	class Module
	{
	public:
		explicit Module(std::shared_ptr<BaseModuleImpl> impl);
		~Module();

	public:
		[[nodiscard]] QString equipmentId() const;

		[[nodiscard]] int lmNumber() const;
		[[nodiscard]] E::Channel channel() const;

		[[nodiscard]] ModuleJumpers moduleJumpers() const;

		[[nodiscard]] DeviceState deviceState() const;

		[[nodiscard]] bool isPowerOff() const;
		void setPowerOff(bool value);

	private:
		std::shared_ptr<BaseModuleImpl> m_impl;
	};
} // namespace Sim