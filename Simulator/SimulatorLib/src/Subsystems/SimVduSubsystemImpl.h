#pragma once
#include "SimSubsystemImpl.h"

#include "../Modules/SimVduModuleImpl.h"
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


	class VduSubsystemImpl : public SubsystemImpl
	{
	public:
		VduSubsystemImpl() = delete;
		VduSubsystemImpl(const VduSubsystemImpl&) = delete;
		VduSubsystemImpl& operator=(const VduSubsystemImpl&) = delete;

		explicit VduSubsystemImpl(QString subsystemId, SimulatorPrivate* simulator);

	public:
		bool load(const Hardware::ModuleFirmware& firmware,
				  const LmDescription& lmDescription,
				  const ConnectionsImpl& connections,
				  const LogicModulesInfo& logicModulesExtraInfo);

	private:
		std::shared_ptr<VduModuleImpl> addDevice(const Hardware::LogicModuleInfo& lm);
		bool removeDevice(QString equipmentId);

	public:
		std::vector<std::shared_ptr<VduModuleImpl>> vduModules();
		std::shared_ptr<VduModuleImpl> vduModules(QString equipmentId);

	private:
		std::map<int, std::shared_ptr<BaseModuleImpl>> m_devicesByLmNumber; // key is LmNumber
	};
} // namespace Sim
