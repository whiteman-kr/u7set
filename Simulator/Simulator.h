#pragma once

#include <memory>
#include <map>
#include <vector>
#include "../lib/ModuleFirmware.h"
#include "../lib/LmDescription.h"
#include "../lib/ILogFile.h"
#include "SimSubsystem.h"
#include "SimControl.h"
#include "SimAppSignalManager.h"
#include "SimTuningSignalManager.h"
#include "SimOverrideSignals.h"
#include "SimConnections.h"
#include "SimScriptSimulator.h"
#include "SimAppDataTransmitter.h"
#include "SimScopedLog.h"


class QTextStream;


namespace Sim
{
	class LogicModule;


	class Simulator : public QObject
	{
		Q_OBJECT

	public:
		explicit Simulator(ILogFile* log, QObject* parent);		// if log is nullptr then log to console
		virtual ~Simulator();

	public:
		bool load(QString buildPath);
		void clear();

		// Flow control
		//
		[[nodiscard]] bool isRunning() const;
		[[nodiscard]] bool isPaused() const;
		[[nodiscard]] bool isStopped() const;

		// Script Tests
		//
		bool runScript(const SimScriptItem& script);				// Starts one script in separate thread and returns immediately
		bool runScripts(const std::vector<SimScriptItem>& scripts);	// Starts a pack of scripts in separate thread and returns immediately
		bool stopScript();											// Stops script if it is running
		bool waitScript(unsigned long msecs = ULONG_MAX);			// Wait script to stop
		bool scriptResult();

	private:
		void clearImpl();
		bool loadFunc(QString buildPath);
		bool loadFirmwares(QString buildPath);
		bool loadLmDescriptions(QString buildPath);
		bool loadConnectionsInfo(QString buildPath);
		bool loadAppSignals(QString buildPath);

	signals:
		void projectUpdated();				// Project was loaded or cleared

		void scriptStarted();
		void scriptFinished();

	public:
		[[nodiscard]] ScopedLog& log();

		[[nodiscard]] bool isLoaded() const;
		[[nodiscard]] QString buildPath() const;

		[[nodiscard]] QString projectName() const;

		[[nodiscard]] const Sim::Connections& connections() const;
		[[nodiscard]] Sim::Connections& connections();

		[[nodiscard]] std::vector<std::shared_ptr<Subsystem>> subsystems() const;
		[[nodiscard]] std::shared_ptr<LogicModule> logicModule(QString equipmentId) const;
		[[nodiscard]] std::vector<std::shared_ptr<LogicModule>> logicModules() const;

		[[nodiscard]] Sim::AppDataTransmitter& appDataTransmitter();
		[[nodiscard]] const Sim::AppDataTransmitter& appDataTransmitter() const;

		[[nodiscard]] Sim::AppSignalManager& appSignalManager();
		[[nodiscard]] const Sim::AppSignalManager& appSignalManager() const;

		[[nodiscard]] Sim::TuningSignalManager& tuningSignalManager();
		[[nodiscard]] const Sim::TuningSignalManager& tuningSignalManager() const;

		[[nodiscard]] Sim::OverrideSignals& overrideSignals();
		[[nodiscard]] const Sim::OverrideSignals& overrideSignals() const;

		[[nodiscard]] Sim::Control& control();
		[[nodiscard]] const Sim::Control& control() const;

	private:
		mutable ScopedLog m_log;

		QString m_buildPath;
		Hardware::ModuleFirmwareStorage m_firmwares;	// Loaded bts file

		Sim::Connections m_connections;

		std::map<QString, std::shared_ptr<LmDescription>> m_lmDescriptions;	// Key is filename
		std::map<QString, std::shared_ptr<Subsystem>> m_subsystems;			// Key is SubsystemID


		// Signals Management
		//
		Sim::AppDataTransmitter m_appDataTransmitter{this};

		Sim::AppSignalManager m_appSignalManager{this};
		Sim::TuningSignalManager m_tuningSignalManager;

		Sim::OverrideSignals m_overrideSignals{this};

		// Control thread
		//
		Sim::Control m_control{this};

		// Scripting/Testing
		//
		ScriptSimulator m_scriptSimulator;
	};
}


