#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <memory>
#include <map>
#include <vector>
#include "../lib/ModuleFirmware.h"
#include "../lib/LmDescription.h"
#include "SimOutput.h"
#include "SimSubsystem.h"

class QTextStream;

namespace Sim
{
	class LogicModule;

	class Simulator : public QObject, protected Output
	{
		Q_OBJECT

	public:
		Simulator();
		virtual ~Simulator();

	public:
		bool load(QString buildPath);
		void clear();

	private:
		void clearImpl();
		bool loadFunc(QString buildPath);
		bool loadFirmwares(QString buildPath);
		bool loadLmDescriptions(QString buildPath);
		bool loadSimulationScripts(QString buildPath);

	signals:
		void projectUpdated();	// Project was loaded or cleared

	public:
		bool isLoaded() const;
		QString buildPath() const;

		std::vector<std::shared_ptr<Subsystem>> subsystems() const;
		std::shared_ptr<LogicModule> logicModule(QString equipmentId);

	private:
		QString m_buildPath;
		Hardware::ModuleFirmwareStorage m_firmwares;	// Loaded bts file

		std::map<QString, std::shared_ptr<LmDescription>> m_lmDescriptions;	// Key is filename
		std::map<QString, QString> m_simScript;								// Key is filename, value is afbl simulation script
		std::map<QString, std::shared_ptr<Subsystem>> m_subsystems;							// Key is SubsystemID
	};
}

#endif // SIMULATOR_H
