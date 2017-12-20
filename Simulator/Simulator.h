#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <map>
#include "../lib/ModuleFirmware.h"
#include "../lib/LmDescription.h"
#include "Output.h"
#include "Subsystem.h"

class QTextStream;

namespace Sim
{
	class Simulator : protected Output
	{
	public:
		// outputStream - stream for console output
		// to out to stdout: [code]QTextStream textStream(stdout);[/code]
		// outputStream can be nullptr
		//
		explicit Simulator(QTextStream* outputStream = nullptr);
		virtual ~Simulator();

	public:
		bool load(QString buildPath);

	private:
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
