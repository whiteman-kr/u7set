#include "SimSubsystemImpl.h"

#include "../SimConnectionsImpl.h"
#include "../SimulatorPrivate.h"
#include "SimLmSubsystemImpl.h"
#include "SimVduSubsystemImpl.h"

#include <CommonLib/ConstStrings.h>
#include <HardwareLib/ModuleFirmware.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <cmath>
#include <limits>

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

	bool SubsystemImpl::loadJumpers()
	{
		const QString fileName = File::SUBSYSTEM_DESC_JSON;
		const QString fullFilePath = QDir{subsystemDir()}.absoluteFilePath(fileName);

		QFile file{fullFilePath};
		if (file.open(QIODevice::ReadOnly) == false)
		{
			m_log.writeError(QObject::tr("Cannot open file %1, SubsystemID %2.").arg(fullFilePath).arg(m_subsystemId));
			return false;
		}

		/*		read such json file:
				{
			"Subsystem": {
				"ID": "SSRVDU",
				"Key": 2,
				"Modules": [
					{
						"EquipmentId": "SYSTEMID_RACKID_CH01_RVDU",
						"Jumpers": "20610",
						"JumpersBin": "0101 0000 1000 0010",
						"JumpersHex": "0x5082",
						"LmNumber": 2
					},
					...
				]
			}
		}
				*/

		QJsonParseError parseError;
		const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);

		if (parseError.error != QJsonParseError::NoError || document.isObject() == false)
		{
			m_log.writeError(
				QObject::tr("Cannot parse %1 file, SubsystemID %2: %3").arg(fileName).arg(m_subsystemId).arg(parseError.errorString()));
			return false;
		}

		const QJsonObject subsystemObject = document.object().value(QLatin1String("Subsystem")).toObject();
		if (subsystemObject.isEmpty() == true || subsystemObject.value(QLatin1String("ID")).toString() != m_subsystemId)
		{
			m_log.writeError(QObject::tr("Invalid subsystem description in %1, SubsystemID %2").arg(fileName).arg(m_subsystemId));
			return false;
		}

		const QJsonValue keyValue = subsystemObject.value(QLatin1String("Key"));
		const double subsystemKey = keyValue.toDouble(-1);
		if (keyValue.isDouble() == false || std::trunc(subsystemKey) != subsystemKey)
		{
			m_log.writeError(QObject::tr("Invalid subsystem key in %1, SubsystemID %2").arg(fileName).arg(m_subsystemId));
			return false;
		}

		const QJsonValue modulesValue = subsystemObject.value(QLatin1String("Modules"));
		if (modulesValue.isArray() == false)
		{
			m_log.writeError(QObject::tr("Modules array is missing in %1, SubsystemID %2").arg(fileName).arg(m_subsystemId));
			return false;
		}

		std::set<QString> loadedEquipmentIds;
		for (const QJsonValue& moduleValue : modulesValue.toArray())
		{
			const QJsonObject moduleObject = moduleValue.toObject();
			const QString equipmentId = moduleObject.value(QLatin1String("EquipmentId")).toString();
			const QJsonValue lmNumberValue = moduleObject.value(QLatin1String("LmNumber"));
			const double lmNumber = lmNumberValue.toDouble(-1);

			bool jumpersOk = false;
			const uint jumpersValue = moduleObject.value(QLatin1String("Jumpers")).toString().toUInt(&jumpersOk, 10);

			if (moduleObject.isEmpty() == true || equipmentId.isEmpty() == true || lmNumberValue.isDouble() == false || lmNumber < 0 ||
				lmNumber > std::numeric_limits<quint16>::max() || std::trunc(lmNumber) != lmNumber || jumpersOk == false ||
				jumpersValue > std::numeric_limits<quint16>::max())
			{
				m_log.writeError(QObject::tr("Invalid module description in %1, SubsystemID %2").arg(fileName).arg(m_subsystemId));
				return false;
			}

			auto device = m_devicesByEquipmentId.find(equipmentId);
			if (device == m_devicesByEquipmentId.end())
			{
				continue;
			}

			if (device->second->lmNumber() != static_cast<int>(lmNumber) || loadedEquipmentIds.insert(equipmentId).second == false)
			{
				m_log.writeError(
					QObject::tr("Module description does not match loaded module %1, SubsystemID %2").arg(equipmentId).arg(m_subsystemId));
				return false;
			}

			device->second->setModuleJumpers(
				{m_subsystemId, static_cast<quint16>(subsystemKey), static_cast<quint16>(lmNumber), static_cast<quint16>(jumpersValue)});
		}

		if (loadedEquipmentIds.size() != m_devicesByEquipmentId.size())
		{
			m_log.writeError(QObject::tr("Missing module descriptions in %1, SubsystemID %2").arg(fileName).arg(m_subsystemId));
			return false;
		}

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

	QString SubsystemImpl::subsystemDir() const
	{
		QString subsystemPath = m_simulator->buildPath() + QDir::separator() + "Subsystems" + QDir::separator() + m_subsystemId;
		return QDir{subsystemPath}.path();
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