#include "SimLmSubsystemImpl.h"

#include "../SimConnectionsImpl.h"
#include "../SimulatorPrivate.h"

#include <HardwareLib/ModuleFirmware.h>

namespace Sim
{
	//
	// LmSubsystemImpl
	//
	LmSubsystemImpl::LmSubsystemImpl(QString subsystemId, SimulatorPrivate* simulator) :
		SubsystemImpl{subsystemId, simulator}
	{
	}

	bool LmSubsystemImpl::load(const Hardware::ModuleFirmware& firmware,
							   const LmDescription& lmDescription,
							   const ConnectionsImpl& connections,
							   const LogicModulesInfo& logicModulesExtraInfo)
	{
		if (firmware.uartExists(static_cast<int>(UartId::ApplicationLogic)) == false)
		{
			m_log.writeError(QObject::tr("Application data is not found in firmware."));
			return false;
		}

		if (firmware.uartExists(static_cast<int>(UartId::Configuration)) == false)
		{
			m_log.writeError(QObject::tr("Configuration data is not found in firmware."));
			return false;
		}

		if (firmware.uartExists(static_cast<int>(UartId::Tuning)) == false)
		{
			m_log.writeError(QObject::tr("Tuning data is not found in firmware."));
			return false;
		}

		const std::vector<Hardware::LogicModuleInfo>& lms = firmware.logicModulesInfo();
		if (lms.empty() == true)
		{
			return true;
		}

		for (const Hardware::LogicModuleInfo& lmInfo : lms)
		{
			m_log.writeMessage(QObject::tr("Load firmware for LogicModule %1").arg(lmInfo.equipmentId));

			std::shared_ptr<LogicModuleImpl> logicModule = std::make_shared<LogicModuleImpl>(m_simulator);

			if (bool loadOk = logicModule->load(lmInfo, lmDescription, firmware, connections, logicModulesExtraInfo); //
				loadOk == false)
			{
				return false;
			}

			if (logicModule->appCommands().empty() == true)
			{
				continue;
			}

			if (bool deviceAdded = addDevice(logicModule); //
				deviceAdded == false)
			{
				return false;
			}
		}

		if (loadJumpers() == false)
		{
			return false;
		}

		return true;
	}

	bool LmSubsystemImpl::addDevice(std::shared_ptr<LogicModuleImpl> module)
	{
		assert(module);

		if (auto p = m_devicesByLmNumber.try_emplace(module->lmNumber(), module); //
			p.second == false)
		{
			m_log.writeError(QObject::tr("Cannot add device, device with LmNumber %1 already exists.").arg(module->lmNumber()));
			return false;
		}

		bool added = SubsystemImpl::addDevice(module);
		if (added == false)
		{
			m_devicesByLmNumber.erase(module->lmNumber()); // Device to m_devicesByLmNumber already was added, remove it
			assert(m_devicesByLmNumber.size() == m_devicesByEquipmentId.size());
			return false;
		}

		assert(m_devicesByLmNumber.size() == m_devicesByEquipmentId.size());
		return true;
	}

	bool LmSubsystemImpl::removeDevice(QString equipmentId)
	{
		assert(m_devicesByLmNumber.size() == m_devicesByEquipmentId.size());

		auto it = m_devicesByEquipmentId.find(equipmentId);
		if (it == m_devicesByEquipmentId.end())
		{
			m_log.writeError(QObject::tr("Cannot remove device, device with EquipmentID %1 not found.").arg(equipmentId));
			return false;
		}

		std::shared_ptr<BaseModuleImpl> device = it->second;

		SubsystemImpl::removeDevice(equipmentId);
		size_t removed = m_devicesByLmNumber.erase(device->logicModuleInfo().lmNumber);

		if (removed == 0)
		{
			m_log.writeError(
				QObject::tr("Cannot remove device, device with LmNumber %1 not found.").arg(device->logicModuleInfo().lmNumber));
			return false;
		}

		assert(m_devicesByLmNumber.size() == m_devicesByEquipmentId.size());
		return true;
	}

	std::vector<std::shared_ptr<LogicModuleImpl>> LmSubsystemImpl::logicModules()
	{
		std::vector<std::shared_ptr<LogicModuleImpl>> result;
		result.reserve(m_devicesByLmNumber.size());

		for (auto [_, device] : m_devicesByEquipmentId)
		{
			auto asLm = std::dynamic_pointer_cast<LogicModuleImpl>(device);
			assert(asLm);
			result.push_back(asLm);
		}

		std::stable_sort(result.begin(),
						 result.end(),
						 [](const auto& a, const auto& b)
						 {
							 return a->equipmentId() < b->equipmentId();
						 });

		return result;
	}

	std::shared_ptr<LogicModuleImpl> LmSubsystemImpl::logicModule(QString equipmentId)
	{
		auto it = m_devicesByEquipmentId.find(equipmentId);
		if (it == m_devicesByEquipmentId.end())
		{
			return {};
		}
		else
		{
			auto asLm = std::dynamic_pointer_cast<LogicModuleImpl>(it->second);
			assert(asLm);
			return asLm;
		}
	}
} // namespace Sim
