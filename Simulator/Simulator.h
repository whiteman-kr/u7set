#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <map>
#include "../lib/ModuleFirmware.h"
#include "../lib/LmDescription.h"
#include "SimOutput.h"
#include "SimSubsystem.h"

class QTextStream;

namespace Sim
{
	class LogicModule;

	class Simulator : protected Output
	{
	public:
		Simulator();
		virtual ~Simulator();

	public:
		bool load(QString buildPath);
		void clear();

	private:
		bool loadFunc(QString buildPath);
		bool loadFirmwares(QString buildPath);
		bool loadLmDescriptions(QString buildPath);
		bool loadSimulationScripts(QString buildPath);

	public:
		std::shared_ptr<LogicModule> logicModule(QString equipmentId);

	private:
		Hardware::ModuleFirmwareStorage m_firmwares;	// Loaded bts file

		std::map<QString, std::shared_ptr<LmDescription>> m_lmDescriptions;	// Key is filename
		std::map<QString, QString> m_simScript;								// Key is filename, value is afbl simulation script
		std::map<QString, Subsystem> m_subsystems;							// Key is SubsystemID
	};

}

#endif // SIMULATOR_H
