#pragma once

#include "SimSubsystemImpl.h"

#include "../Modules/SimLogicModuleImpl.h"
#include "../SimScopedLog.h"


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
		bool addDevice(std::shared_ptr<LogicModuleImpl> module);
		bool removeDevice(QString equipmentId);

	public:
		std::vector<std::shared_ptr<LogicModuleImpl>> logicModules();
		std::shared_ptr<LogicModuleImpl> logicModule(QString equipmentId);

	private:
		std::map<int, std::shared_ptr<BaseModuleImpl>> m_devicesByLmNumber; // key is LmNumber
	};
} // namespace Sim
