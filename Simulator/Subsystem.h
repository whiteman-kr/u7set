#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H
#include <map>
#include <memory>
#include "Output.h"
#include "Eeprom.h"
#include "LmModel.h"

namespace Hardware
{
	class ModuleFirmware;
	struct LogicModuleInfo;
}

namespace Sim
{
	class Subsystem : protected Output
	{
	public:
		Subsystem() = delete;
		Subsystem(const Subsystem&) = delete;
		Subsystem(QString subsystemId, const Output& output);
		Subsystem& operator=(const Subsystem&) = delete;

	public:
		bool load(const Hardware::ModuleFirmware& firmware, const LmDescription& lmDescription, const QString simulationScript);

	private:
		std::vector<Hardware::LogicModuleInfo> logicModulesInfo() const;

		 std::shared_ptr<LogicModule> addDevice(const Hardware::LogicModuleInfo& lm);
		 bool removeDevice(QString equipmentId);

	private:
		QString m_susystemId;

		std::map<int, std::shared_ptr<LogicModule>> m_devicesByLmNumber;			// key is LmNumber
		std::map<QString, std::shared_ptr<LogicModule>> m_devicesByEquipmentId;		// key is EquipmentID
	};
}

#endif // SUBSYSTEM_H
