#include <SimulatorLib/SimLogicModule.h>
#include "SimLogicModuleImpl.h"

namespace Sim
{
	LogicModule::LogicModule(std::shared_ptr<LogicModuleImpl> impl) :
		m_impl{std::move(impl)}
	{
		Q_ASSERT(m_impl);
	}

	LogicModule::~LogicModule() = default;

	QString LogicModule::equipmentId() const
	{
		return m_impl->equipmentId();
	}

	int LogicModule::lmNumber() const
	{
		return m_impl->lmNumber();
	}

	E::Channel LogicModule::channel() const
	{
		return m_impl->channel();
	}

	const ::LogicModuleInfo& LogicModule::logicModuleExtraInfo() const
	{
		return m_impl->logicModuleExtraInfo();
	}

	void LogicModule::setLogicModuleExtraInfo(const ::LogicModuleInfo& value)
	{
		return m_impl->setLogicModuleExtraInfo(value);
	}

	const std::vector<DeviceCommand>& LogicModule::appCommands() const
	{
		return m_impl->appCommands();
	}

	std::unordered_map<int, size_t> LogicModule::offsetToCommand() const
	{
		return m_impl->offsetToCommand();
	}

	const DeviceCommand& LogicModule::offsetToCommand(int offset) const
	{
		return m_impl->offsetToCommand(offset);
	}

	RuntimeMode LogicModule::runtimeMode() const
	{
		return m_impl->runtimeMode();
	}

	DeviceState LogicModule::deviceState() const
	{
		return m_impl->deviceState();
	}

	bool LogicModule::isPowerOff() const
	{
		return m_impl->isPowerOff();
	}

	void LogicModule::setPowerOff(bool value)
	{
		return m_impl->setPowerOff(value);
	}

	bool LogicModule::armingKey() const
	{
		return m_impl->armingKey();
	}

	void LogicModule::setArmingKey(bool value)
	{
		return m_impl->setArmingKey(value);
	}

	bool LogicModule::tuningKey() const
	{
		return m_impl->tuningKey();
	}

	void LogicModule::setTuningKey(bool value)
	{
		return m_impl->setTuningKey(value);
	}

	bool LogicModule::sorIsSet() const
	{
		return m_impl->sorIsSet();
	}

	bool LogicModule::sorSetSwitch1() const
	{
		return m_impl->sorSetSwitch1();
	}

	void LogicModule::setSorSetSwitch1(bool value)
	{
		return m_impl->setSorSetSwitch1(value);
	}

	bool LogicModule::sorSetSwitch2() const
	{
		return m_impl->sorSetSwitch2();
	}

	void LogicModule::setSorSetSwitch2(bool value)
	{
		return m_impl->setSorSetSwitch2(value);
	}

	bool LogicModule::sorSetSwitch3() const
	{
		return m_impl->sorSetSwitch3();
	}

	void LogicModule::setSorSetSwitch3(bool value)
	{
		return m_impl->setSorSetSwitch3(value);
	}

	bool LogicModule::testSorResetSwitch(bool newValue)
	{
		return m_impl->testSorResetSwitch(newValue);
	}
} // namespace Sim
