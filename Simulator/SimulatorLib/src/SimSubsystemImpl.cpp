#include "SimSubsystemImpl.h"
#include "SimConnectionsImpl.h"
#include "SimLmSubsystemImpl.h"
#include "SimulatorPrivate.h"

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

	std::vector<Hardware::LogicModuleInfo> SubsystemImpl::modulesInfo() const
	{
		assert(m_devicesByLmNumber.size() == m_devicesByEquipmentId.size());

		std::vector<Hardware::LogicModuleInfo> result;
		result.reserve(m_devicesByLmNumber.size());

		for (auto dev : m_devicesByLmNumber)
		{
			std::shared_ptr<BaseModuleImpl> device = dev.second;
			result.push_back(device->logicModuleInfo());
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
		return false; // TODO: Implement VDU subsystem check if needed
	}

	LmSubsystemImpl* SubsystemImpl::toLmSubsystem()
	{
		return dynamic_cast<LmSubsystemImpl*>(this);
	}

	const LmSubsystemImpl* SubsystemImpl::toLmSubsystem() const
	{
		return dynamic_cast<const LmSubsystemImpl*>(this);
	}

	std::vector<std::shared_ptr<BaseModuleImpl>> SubsystemImpl::modules()
	{
		std::vector<std::shared_ptr<BaseModuleImpl>> result;
		result.reserve(m_devicesByLmNumber.size());

		for (auto& [key, lm] : m_devicesByEquipmentId)
		{
			Q_UNUSED(key);
			result.push_back(lm);
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