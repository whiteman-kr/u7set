#pragma once

#include "SimScopedLog.h"
#include "SimLogicModuleImpl.h"

namespace Hardware
{
	class ModuleFirmware;
	struct LogicModuleInfo;
}

namespace Sim
{
	class SimulatorPrivate;
	class ConnectionsImpl;

	class SubsystemImpl
	{
	public:
		SubsystemImpl() = delete;
		SubsystemImpl(const SubsystemImpl&) = delete;
		SubsystemImpl& operator=(const SubsystemImpl&) = delete;

		explicit SubsystemImpl(QString subsystemId, SimulatorPrivate* simulator);

	public:
		bool load(const Hardware::ModuleFirmware& firmware,
				  const LmDescription& lmDescription,
				  const ConnectionsImpl& connections,
				  const LogicModulesInfo& logicModulesExtraInfo);

	private:
		std::vector<Hardware::LogicModuleInfo> logicModulesInfo() const;

		std::shared_ptr<LogicModuleImpl> addDevice(const Hardware::LogicModuleInfo& lm);
		bool removeDevice(QString equipmentId);

	public:
		QString subsystemId() const;

		std::vector<std::shared_ptr<LogicModuleImpl>> logicModules();
		std::shared_ptr<LogicModuleImpl> logicModule(QString equipmentId);

	private:
		SimulatorPrivate* m_simulator = nullptr;
		mutable ScopedLog m_log;

		QString m_subsystemId;
		LmDescription m_lmDescription;

		std::map<int, std::shared_ptr<LogicModuleImpl>> m_devicesByLmNumber;			// key is LmNumber
		std::map<QString, std::shared_ptr<LogicModuleImpl>> m_devicesByEquipmentId;		// key is EquipmentID
	};
}

