#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H
#include <map>
#include <memory>
#include "SimOutput.h"
#include "SimEeprom.h"
#include "SimLmModel.h"

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
		explicit Subsystem(QString subsystemId);
		Subsystem& operator=(const Subsystem&) = delete;

	public:
		bool load(const Hardware::ModuleFirmware& firmware, const LmDescription& lmDescription, const QString simulationScript);

	private:
		std::vector<Hardware::LogicModuleInfo> logicModulesInfo() const;

		 std::shared_ptr<LogicModule> addDevice(const Hardware::LogicModuleInfo& lm);
		 bool removeDevice(QString equipmentId);

	public:
		 QString subsystemId() const;

		 std::vector<std::shared_ptr<LogicModule>> logicModules();
		 std::shared_ptr<LogicModule> logicModule(QString equipmentId);

	private:
		QString m_subsystemId;

		std::map<int, std::shared_ptr<LogicModule>> m_devicesByLmNumber;			// key is LmNumber
		std::map<QString, std::shared_ptr<LogicModule>> m_devicesByEquipmentId;		// key is EquipmentID
	};
}

#endif // SUBSYSTEM_H
