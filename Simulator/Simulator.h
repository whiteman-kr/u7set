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
		explicit Simulator(QTextStream* outputStream);
		virtual ~Simulator();

	public:
		bool load(QString buildPath);

	private:
		bool loadFirmwares(QString buildPath);
		bool loadLmDescriptions(QString buildPath);

	private:
		Hardware::ModuleFirmwareStorage m_firmwares;	// Loaded bts file

		std::map<QString, std::shared_ptr<LmDescription>> m_lmDescriptions;
		std::map<QString, Subsystem> m_subsystems;		// Key is subsystemId;
	};

}

#endif // SIMULATOR_H
