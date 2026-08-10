#pragma once

#include "SimLogicModuleImpl.h"
#include "SimScopedLog.h"
#include "SimSubsystemImpl.h"

namespace Hardware
{
	class ModuleFirmware;
	struct LogicModuleInfo;
} // namespace Hardware

namespace Sim
{
	class SimulatorPrivate;
	class ConnectionsImpl;


	class LmSubsystemImpl : public SubsystemImpl
	{
	public:
		LmSubsystemImpl() = delete;
		LmSubsystemImpl(const LmSubsystemImpl&) = delete;
		LmSubsystemImpl& operator=(const LmSubsystemImpl&) = delete;

		explicit LmSubsystemImpl(QString subsystemId, SimulatorPrivate* simulator);

	public:
		bool load(const Hardware::ModuleFirmware& firmware,
				  const LmDescription& lmDescription,
				  const ConnectionsImpl& connections,
				  const LogicModulesInfo& logicModulesExtraInfo);

	private:
		std::shared_ptr<LogicModuleImpl> addDevice(const Hardware::LogicModuleInfo& lm);
		bool removeDevice(QString equipmentId);

	public:
		std::vector<std::shared_ptr<LogicModuleImpl>> logicModules();
		std::shared_ptr<LogicModuleImpl> logicModule(QString equipmentId);
	};
} // namespace Sim
