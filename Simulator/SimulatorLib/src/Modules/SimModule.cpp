#include <SimulatorLib/SimModule.h>

#include "SimBaseModuleImpl.h"


namespace Sim
{
	Module::Module(std::shared_ptr<BaseModuleImpl> impl) :
		m_impl{std::move(impl)}
	{
		Q_ASSERT(m_impl);
	}

	Module::~Module() = default;

	QString Module::equipmentId() const
	{
		return m_impl->equipmentId();
	}

	int Module::lmNumber() const
	{
		return m_impl->lmNumber();
	}

	E::Channel Module::channel() const
	{
		return m_impl->channel();
	}

	ModuleJumpers Module::moduleJumpers() const
	{
		return m_impl->moduleJumpers();
	}

	DeviceState Module::deviceState() const
	{
		return m_impl->deviceState();
	}

	bool Module::isPowerOff() const
	{
		return m_impl->isPowerOff();
	}

	void Module::setPowerOff(bool value)
	{
		return m_impl->setPowerOff(value);
	}
} // namespace Sim
