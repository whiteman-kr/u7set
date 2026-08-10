#pragma once

#include "SimLogicModuleImpl.h"
#include "SimScopedLog.h"

namespace Hardware
{
	class ModuleFirmware;
	struct LogicModuleInfo;
} // namespace Hardware

namespace Sim
{
	class ConnectionsImpl;
	class LmSubsystemImpl;
	class SimulatorPrivate;

	class SubsystemImpl
	{
	public:
		SubsystemImpl() = delete;
		SubsystemImpl(const SubsystemImpl&) = delete;
		virtual ~SubsystemImpl() = default;

		SubsystemImpl& operator=(const SubsystemImpl&) = delete;

		explicit SubsystemImpl(QString subsystemId, SimulatorPrivate* simulator);

	private:
		std::vector<Hardware::LogicModuleInfo> modulesInfo() const;

	public:
		QString subsystemId() const;

		bool isLmSubsystem() const;
		bool isVduSubsystem() const;

		LmSubsystemImpl* toLmSubsystem();
		const LmSubsystemImpl* toLmSubsystem() const;

		std::vector<std::shared_ptr<BaseModuleImpl>> modules();
		std::shared_ptr<BaseModuleImpl> module(QString equipmentId);

	protected:
		SimulatorPrivate* m_simulator = nullptr;
		mutable ScopedLog m_log;

		QString m_subsystemId;

		std::map<int, std::shared_ptr<BaseModuleImpl>> m_devicesByLmNumber;        // key is LmNumber
		std::map<QString, std::shared_ptr<BaseModuleImpl>> m_devicesByEquipmentId; // key is EquipmentID
	};
} // namespace Sim
