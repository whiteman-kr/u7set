#include "SimSubsystemImpl.h"

#include "../SimConnectionsImpl.h"
#include "../SimulatorPrivate.h"
#include "SimLmSubsystemImpl.h"
#include "SimVduSubsystemImpl.h"

#include <HardwareLib/ModuleFirmware.h>

namespace Sim
{
	//
	// Subsystem
	//
	SubsystemImpl::SubsystemImpl(QString subsystemId, SimulatorPrivate* simulator) :
		m_simulator(simulator),
		m_log(simulator->log(), subsystemId),
		m_subsystemId(subsystemId)
	{
	}

	bool SubsystemImpl::addDevice(std::shared_ptr<BaseModuleImpl> module)
	{
		if (module == nullptr)
		{
			assert(false);
			m_log.writeError(QObject::tr("Internal error: Cannot add device, module is nullptr, SubsystemID %1").arg(m_subsystemId));
			return false;
		}

		if (auto p = m_devicesByEquipmentId.try_emplace(module->equipmentId(), module); //
			p.second == false)
		{
			m_log.writeError(QObject::tr("Cannot add device, device with EquipmentID %1 already exists.").arg(module->equipmentId()));
			return false;
		}

		return true;
	}

	bool SubsystemImpl::removeDevice(QString equipmentId)
	{
		if (m_devicesByEquipmentId.contains(equipmentId) == false)
		{
			m_log.writeError(QObject::tr("Cannot remove device, device with EquipmentID %1 not found.").arg(equipmentId));
			return false;
		}

		m_devicesByEquipmentId.erase(equipmentId);
		return true;
	}

	std::vector<Hardware::LogicModuleInfo> SubsystemImpl::modulesInfo() const
	{
		std::vector<Hardware::LogicModuleInfo> result;
		result.reserve(m_devicesByEquipmentId.size());

		for (auto& [_, module] : m_devicesByEquipmentId)
		{
			assert(module);
			result.push_back(module->logicModuleInfo());
		}

		return result;
	}

	QString SubsystemImpl::subsystemId() const
	{
		return m_subsystemId;
	}

	bool SubsystemImpl::isLmSubsystem() const
	{
		return dynamic_cast<const LmSubsystemImpl*>(this) != nullptr;
	}

	bool SubsystemImpl::isVduSubsystem() const
	{
		return dynamic_cast<const VduSubsystemImpl*>(this) != nullptr;
	}

	LmSubsystemImpl* SubsystemImpl::toLmSubsystem()
	{
		return dynamic_cast<LmSubsystemImpl*>(this);
	}

	const LmSubsystemImpl* SubsystemImpl::toLmSubsystem() const
	{
		return dynamic_cast<const LmSubsystemImpl*>(this);
	}

	VduSubsystemImpl* SubsystemImpl::toVduSubsystem()
	{
		return dynamic_cast<VduSubsystemImpl*>(this);
	}

	const VduSubsystemImpl* SubsystemImpl::toVduSubsystem() const
	{
		return dynamic_cast<const VduSubsystemImpl*>(this);
	}

	std::vector<std::shared_ptr<BaseModuleImpl>> SubsystemImpl::modules()
	{
		std::vector<std::shared_ptr<BaseModuleImpl>> result;
		result.reserve(m_devicesByEquipmentId.size());

		for (auto& [_, module] : m_devicesByEquipmentId)
		{
			result.push_back(module);
		}

		std::sort(result.begin(),
				  result.end(),
				  [](const auto& a, const auto& b)
				  {
					  return a->equipmentId() < b->equipmentId();
				  });

		return result;
	}

	std::shared_ptr<BaseModuleImpl> SubsystemImpl::module(QString equipmentId)
	{
		auto it = m_devicesByEquipmentId.find(equipmentId);

		if (it == m_devicesByEquipmentId.end())
		{
			return {};
		}
		else
		{
			return it->second;
		}
	}

} // namespace Sim