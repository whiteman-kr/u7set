#pragma once

#include "SimDeviceState.h"

#include <HardwareLib/LogicModulesInfo.h>
#include <SimulatorLib/SimModuleJumpers.h>

#include <memory>


namespace Sim
{
	class VduModuleImpl;

	class VduModule
	{
	public:
		explicit VduModule(std::shared_ptr<VduModuleImpl> impl);
		~VduModule();

	public:
		[[nodiscard]] QString equipmentId() const;

		[[nodiscard]] int lmNumber() const;
		[[nodiscard]] E::Channel channel() const;

		[[nodiscard]] ModuleJumpers moduleJumpers() const;

		const ::LogicModuleInfo& logicModuleExtraInfo() const;
		void setLogicModuleExtraInfo(const ::LogicModuleInfo& value);

		[[nodiscard]] QString sharedMemoryKey() const;

		[[nodiscard]] RuntimeMode runtimeMode() const;
		[[nodiscard]] DeviceState deviceState() const;

		[[nodiscard]] bool isPowerOff() const;
		void setPowerOff(bool value);

#if 0
		[[nodiscard]] bool armingKey() const;
		void setArmingKey(bool value);

		[[nodiscard]] bool tuningKey() const;
		void setTuningKey(bool value);
#endif

	private:
		std::shared_ptr<VduModuleImpl> m_impl;
	};
} // namespace Sim