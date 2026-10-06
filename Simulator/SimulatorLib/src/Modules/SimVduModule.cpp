#include <SimulatorLib/SimVduModule.h>

#include "SimVduModuleImpl.h"


namespace Sim
{
	VduModule::VduModule(std::shared_ptr<VduModuleImpl> impl) :
		m_impl{std::move(impl)}
	{
		Q_ASSERT(m_impl);
	}

	VduModule::~VduModule() = default;

	QString VduModule::equipmentId() const
	{
		return m_impl->equipmentId();
	}

	int VduModule::lmNumber() const
	{
		return m_impl->lmNumber();
	}

	E::Channel VduModule::channel() const
	{
		return m_impl->channel();
	}

	ModuleJumpers VduModule::moduleJumpers() const
	{
		return m_impl->moduleJumpers();
	}

	const ::LogicModuleInfo& VduModule::logicModuleExtraInfo() const
	{
		return m_impl->logicModuleExtraInfo();
	}

	void VduModule::setLogicModuleExtraInfo(const ::LogicModuleInfo& value)
	{
		return m_impl->setLogicModuleExtraInfo(value);
	}

	QString VduModule::sharedMemoryKey() const
	{
		return m_impl->sharedMemoryKey();
	}

	RuntimeMode VduModule::runtimeMode() const
	{
		return m_impl->runtimeMode();
	}

	DeviceState VduModule::deviceState() const
	{
		return m_impl->deviceState();
	}

	bool VduModule::isPowerOff() const
	{
		return m_impl->isPowerOff();
	}

	void VduModule::setPowerOff(bool value)
	{
		return m_impl->setPowerOff(value);
	}

	bool VduModule::startSimAppProcess(const QString& simAppPath)
	{
		return m_impl->startSimAppProcess(simAppPath);
	}

	QStringList VduModule::simAppArguments() const
	{
		return m_impl->simAppArguments();
	}

	bool VduModule::discreteInputState(size_t index) const
	{
		return m_impl->discreteInputState(index);
	}

	void VduModule::setDiscreteInputState(size_t index, bool value)
	{
		return m_impl->setDiscreteInputState(index, value);
	}

	bool VduModule::discreteInputValidity(size_t index) const
	{
		return m_impl->discreteInputValidity(index);
	}

	void VduModule::setDiscreteInputValidity(size_t index, bool value)
	{
		return m_impl->setDiscreteInputValidity(index, value);
	}

	std::optional<bool> VduModule::discreteOutputState(size_t index) const
	{
		return m_impl->discreteOutputState(index);
	}

	std::array<std::optional<bool>, SimVduSharedData::DiscreteOutputCount> VduModule::discreteOutputStates() const
	{
		return m_impl->discreteOutputStates();
	}

	void VduModule::setDiscreteOutputState(size_t index, std::optional<bool> value)
	{
		return m_impl->setDiscreteOutputState(index, value);
	}

} // namespace Sim
