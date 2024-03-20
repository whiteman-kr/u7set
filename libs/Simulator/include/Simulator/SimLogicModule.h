#pragma once

#include "SimDeviceCommand.h"
#include "SimDeviceState.h"

#include <HardwareLib/LogicModulesInfo.h>

namespace Sim
{
	class LogicModuleImpl;

	class LogicModule
	{
	public:
		explicit LogicModule(std::shared_ptr<LogicModuleImpl> impl);
		~LogicModule();

	public:
		// bool receiveConnectionsData(std::chrono::microseconds currentTime);

	public:
		QString equipmentId() const;
		int lmNumber() const;
		E::Channel channel() const;

		const ::LogicModuleInfo& logicModuleExtraInfo() const;
		void setLogicModuleExtraInfo(const ::LogicModuleInfo& value);

		const std::vector<DeviceCommand>& appCommands() const;

		std::unordered_map<int, size_t> offsetToCommand() const;
		const DeviceCommand& offsetToCommand(int offset) const;

		[[nodiscard]] RuntimeMode runtimeMode() const;

		[[nodiscard]] DeviceState deviceState() const;

		[[nodiscard]] bool isPowerOff() const;
		void setPowerOff(bool value);

		[[nodiscard]] bool armingKey() const;
		void setArmingKey(bool value);

		[[nodiscard]] bool tuningKey() const;
		void setTuningKey(bool value);

		[[nodiscard]] bool sorIsSet() const;

		[[nodiscard]] bool sorSetSwitch1() const;
		void setSorSetSwitch1(bool value);

		[[nodiscard]] bool sorSetSwitch2() const;
		void setSorSetSwitch2(bool value);

		[[nodiscard]] bool sorSetSwitch3() const;
		void setSorSetSwitch3(bool value);

		[[nodiscard]] bool testSorResetSwitch(bool newValue);

	private:
		std::shared_ptr<LogicModuleImpl> m_impl;
	};
} // namespace Sim