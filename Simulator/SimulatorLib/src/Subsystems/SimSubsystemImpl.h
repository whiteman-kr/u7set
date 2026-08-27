#pragma once

#include "../Modules/SimBaseModuleImpl.h"
#include "../SimScopedLog.h"

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
	class VduSubsystemImpl;


	class SubsystemImpl
	{
	public:
		SubsystemImpl() = delete;
		SubsystemImpl(const SubsystemImpl&) = delete;
		virtual ~SubsystemImpl() = default;

		SubsystemImpl& operator=(const SubsystemImpl&) = delete;

		explicit SubsystemImpl(QString subsystemId, SimulatorPrivate* simulator);

	protected:
		bool addDevice(std::shared_ptr<BaseModuleImpl> module);
		bool removeDevice(QString equipmentId);

		bool loadJumpers(); // Load jumpers values from Description.json

	private:
		std::vector<Hardware::LogicModuleInfo> modulesInfo() const;

	public:
		QString subsystemId() const;

		QString subsystemDir() const;

		bool isLmSubsystem() const;
		bool isVduSubsystem() const;

		LmSubsystemImpl* toLmSubsystem();
		const LmSubsystemImpl* toLmSubsystem() const;

		VduSubsystemImpl* toVduSubsystem();
		const VduSubsystemImpl* toVduSubsystem() const;

		std::vector<std::shared_ptr<BaseModuleImpl>> modules();
		std::shared_ptr<BaseModuleImpl> module(QString equipmentId);

	protected:
		SimulatorPrivate* m_simulator = nullptr;
		mutable ScopedLog m_log;

		QString m_subsystemId;

		std::map<QString, std::shared_ptr<BaseModuleImpl>> m_devicesByEquipmentId; // key is EquipmentID
	};
} // namespace Sim
