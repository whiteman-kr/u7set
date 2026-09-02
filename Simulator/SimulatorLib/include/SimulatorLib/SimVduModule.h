#pragma once

#include "SimDeviceState.h"

#include <HardwareLib/LogicModulesInfo.h>
#include <SimulatorLib/SimModuleJumpers.h>
#include <SimulatorLib/SimVduSharedData.h>

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

		bool startSimAppProcess(const QString& simAppPath);
		QStringList simAppArguments() const;

		// External discrete inputs and outputs
		// SimulatorLib provides inputs with validities,
		// Vdu Sim App provides outputs.
		//
	public:
		[[nodiscard]] bool discreteInputState(size_t index) const;
		void setDiscreteInputState(size_t index, bool value);

		[[nodiscard]] bool discreteInputValidity(size_t index) const;
		void setDiscreteInputValidity(size_t index, bool value);

		[[nodiscard]] std::optional<bool> discreteOutputState(size_t index) const;
		[[nodiscard]] std::array<std::optional<bool>, SimVduSharedData::DiscreteOutputCount> discreteOutputStates() const;
		void setDiscreteOutputState(size_t index, std::optional<bool> value);

	private:
		std::shared_ptr<VduModuleImpl> m_impl;
	};
} // namespace Sim