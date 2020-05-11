#pragma once

#include <memory>
#include <map>
#include <vector>
#include "../lib/ModuleFirmware.h"
#include "../lib/LmDescription.h"
#include "SimOutput.h"
#include "SimSubsystem.h"
#include "SimControl.h"
#include "SimAppSignalManager.h"
#include "SimTuningSignalManager.h"
#include "SimOverrideSignals.h"
#include "SimConnections.h"

class QTextStream;

namespace Sim
{
	class LogicModule;

	class Simulator : public QObject, protected Output
	{
		Q_OBJECT

	public:
		explicit Simulator(QObject* parent = nullptr);
		virtual ~Simulator();

	public:
		bool load(QString buildPath);
		void clear();

		bool isRunning() const;
		bool isPaused() const;
		bool isStopped() const;

	private:
		void clearImpl();
		bool loadFunc(QString buildPath);
		bool loadFirmwares(QString buildPath);
		bool loadLmDescriptions(QString buildPath);
		bool loadConnectionsInfo(QString buildPath);
		bool loadAppSignals(QString buildPath);

	signals:
		void projectUpdated();	// Project was loaded or cleared

	public:
		bool isLoaded() const;
		QString buildPath() const;

		const Sim::Connections& connections() const;

		std::vector<std::shared_ptr<Subsystem>> subsystems() const;
		std::shared_ptr<LogicModule> logicModule(QString equipmentId);
		std::vector<std::shared_ptr<LogicModule>> logicModules();

		Sim::AppSignalManager& appSignalManager();
		const Sim::AppSignalManager& appSignalManager() const;

		Sim::TuningSignalManager& tuningSignalManager();
		const Sim::TuningSignalManager& tuningSignalManager() const;

		Sim::OverrideSignals& overrideSignals();
		const Sim::OverrideSignals& overrideSignals() const;

		Sim::Control& control();
		const Sim::Control& control() const;

	private:
		QString m_buildPath;
		Hardware::ModuleFirmwareStorage m_firmwares;	// Loaded bts file

		Sim::Connections m_connections;

		std::map<QString, std::shared_ptr<LmDescription>> m_lmDescriptions;	// Key is filename
		std::map<QString, std::shared_ptr<Subsystem>> m_subsystems;			// Key is SubsystemID

		// Signals Management
		//
		Sim::AppSignalManager m_appSignalManager{this};
		Sim::TuningSignalManager m_tuningSignalManager;

		Sim::OverrideSignals m_overrideSignals{this};

		// Control thread
		//
		Sim::Control m_control{this};
	};
}


