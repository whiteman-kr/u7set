#include "SimSubsystem.h"
#include "../lib/ModuleFirmware.h"
#include "SimConnections.h"

namespace Sim
{
	//
	// Subsystem
	//
	Subsystem::Subsystem(QString subsystemId) :
		Output(subsystemId),
		m_subsystemId(subsystemId)
	{
	}

	bool Subsystem::load(const Hardware::ModuleFirmware& firmware, const LmDescription& lmDescription, const Connections& connections)
	{
		m_lmDescription = lmDescription;

		if (firmware.uartExists(static_cast<int>(UartId::ApplicationLogic)) == false)
		{
			writeError(QObject::tr("Application data is not found in firmware."));
			return false;
		}

		if (firmware.uartExists(static_cast<int>(UartId::Configuration)) == false)
		{
			writeError(QObject::tr("Configuration data is not found in firmware."));
			return false;
		}

		if (firmware.uartExists(static_cast<int>(UartId::Tuning)) == false)
		{
			writeError(QObject::tr("Tuning data is not found in firmware."));
			return false;
		}

		const std::vector<Hardware::LogicModuleInfo>& lms =  firmware.logicModulesInfo();
		if (lms.empty() == true)
		{
			return true;
		}

		for (const Hardware::LogicModuleInfo& lmInfo : lms)
		{
			std::shared_ptr<LogicModule> logicModule = addDevice(lmInfo);
			if (logicModule == nullptr)
			{
				return false;
			}

			writeMessage(QObject::tr("Load firmware for LogicModule %1").arg(lmInfo.equipmentId));

			if (bool ok = logicModule->load(lmInfo, lmDescription, firmware, connections);
				ok == false)
			{
				// There is no simulation for this file, or it's loading had errors
				//
				removeDevice(lmInfo.equipmentId);
			}

		}

		return true;
	}

	std::vector<Hardware::LogicModuleInfo> Subsystem::logicModulesInfo() const
	{
		assert(m_devicesByLmNumber.size() == m_devicesByEquipmentId.size());

		std::vector<Hardware::LogicModuleInfo> result;
		result.reserve(m_devicesByLmNumber.size());

		for (auto dev : m_devicesByLmNumber)
		{
			std::shared_ptr<LogicModule> device = dev.second;
			result.push_back(device->logicModuleInfo());
		}

		return result;
	}

	 std::shared_ptr<LogicModule> Subsystem::addDevice(const Hardware::LogicModuleInfo& lm)
	 {
		std::shared_ptr<LogicModule> result = std::make_shared<LogicModule>();

		if (auto p = m_devicesByLmNumber.try_emplace(lm.lmNumber, result);
			p.second == false)
		{
			writeError(QObject::tr("Cannot add device, device with LmNumber %1 already exists.").arg(lm.lmNumber));
			return std::shared_ptr<LogicModule>();
		}

		if (auto p = m_devicesByEquipmentId.try_emplace(lm.equipmentId, result);
			p.second == false)
		{
			writeError(QObject::tr("Cannot add device, device with EquipmentID %1 already exists.").arg(lm.equipmentId));
			m_devicesByLmNumber.erase(lm.lmNumber);		// Device to m_devicesByLmNumber already was added, remove it
			return std::shared_ptr<LogicModule>();
		}

		 assert(m_devicesByLmNumber.size() == m_devicesByEquipmentId.size());
		 return result;
	 }

	 bool Subsystem::removeDevice(QString equipmentId)
	 {
		auto it = m_devicesByEquipmentId.find(equipmentId);
		if (it == m_devicesByEquipmentId.end())
		{
			writeError(QObject::tr("Cannot remove device, device with EquipmentID %1 not found.").arg(equipmentId));
			return false;
		}

		std::shared_ptr<LogicModule> device = it->second;

		m_devicesByEquipmentId.erase(equipmentId);
		size_t removed = m_devicesByLmNumber.erase(device->logicModuleInfo().lmNumber);

		if (removed == 0)
		{
			writeError(QObject::tr("Cannot remove device, device with LmNumber %1 not found.").arg(device->logicModuleInfo().lmNumber));
			return false;
		}

		assert(m_devicesByLmNumber.size() == m_devicesByEquipmentId.size());
		return true;
	 }

	 QString Subsystem::subsystemId() const
	 {
		 return m_subsystemId;
	 }

	 std::vector<std::shared_ptr<LogicModule>> Subsystem::logicModules()
	 {
		 std::vector<std::shared_ptr<LogicModule>> result;
		 result.reserve(m_devicesByLmNumber.size());

		 for (auto [key, lm] : m_devicesByEquipmentId)
		 {
			 Q_UNUSED(key);
			 result.push_back(lm);
		 }

		 std::sort(result.begin(), result.end(),
				   [](const auto& a, const auto& b)
				   {
						return a->equipmentId() < b->equipmentId();
				   });

		 return result;
	 }

	 std::shared_ptr<LogicModule> Subsystem::logicModule(QString equipmentId)
	 {
		 auto it = m_devicesByEquipmentId.find(equipmentId);

		 if (it == m_devicesByEquipmentId.end())
		 {
			 return std::shared_ptr<LogicModule>();
		 }
		 else
		 {
			 return it->second;
		 }
	 }

}
