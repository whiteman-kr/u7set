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

	const ::LogicModuleInfo& VduModule::logicModuleExtraInfo() const
	{
		return m_impl->logicModuleExtraInfo();
	}

	void VduModule::setLogicModuleExtraInfo(const ::LogicModuleInfo& value)
	{
		return m_impl->setLogicModuleExtraInfo(value);
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
} // namespace Sim
