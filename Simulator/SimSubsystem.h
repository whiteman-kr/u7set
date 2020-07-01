#pragma once

#include <map>
#include <memory>
#include "SimOutput.h"
#include "SimEeprom.h"
#include "SimLogicModule.h"

namespace Hardware
{
	class ModuleFirmware;
	struct LogicModuleInfo;
}

namespace Sim
{
	class Connections;


	class Subsystem : protected Output
	{
	public:
		Subsystem() = delete;
		Subsystem(const Subsystem&) = delete;
		explicit Subsystem(QString subsystemId);
		Subsystem& operator=(const Subsystem&) = delete;

	public:
		bool load(const Hardware::ModuleFirmware& firmware,
				  const LmDescription& lmDescription,
				  const Connections& connections);

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
		LmDescription m_lmDescription;

		std::map<int, std::shared_ptr<LogicModule>> m_devicesByLmNumber;			// key is LmNumber
		std::map<QString, std::shared_ptr<LogicModule>> m_devicesByEquipmentId;		// key is EquipmentID
	};
}

