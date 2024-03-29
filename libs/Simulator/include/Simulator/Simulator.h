#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "../AppSignalLib/ITuningSignalManager.h"
#include "../UtilsLib/ILogFile.h"

#include "SimAppSignalManager.h"
#include "SimConnections.h"
#include "SimControl.h"
#include "SimLogicModule.h"
#include "SimOverrideSignals.h"
#include "SimProfiles.h"
#include "SimScriptItem.h"
#include "SimSoftware.h"
#include "SimSubsystem.h"


namespace Sim
{
	class SimulatorPrivate;

	class Simulator : public QObject
	{
		Q_OBJECT

	public:
		explicit Simulator(ILogFile* log, bool allowDebugMessages, QObject* parent); // if log is nullptr then log to console
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

		/// @brief Starts one script in separate thread and returns immediately.
		///
		bool runScript(const SimScriptItem& script, const SimScriptItem& globalScript, qint64 timeout);

		/// @brief Starts a pack of scripts in separate thread and returns immediately.
		///
		bool runScripts(const std::vector<SimScriptItem>& scripts, const SimScriptItem& globalScript, qint64 timeout);

		/// @brief Stops script if it is running.
		///
		bool stopScript();

		/// @brief Wait script to stop.
		///
		bool waitScript(unsigned long msecs = ULONG_MAX);

		bool scriptResult();

		bool checkSkipOnBuildConst() const;
		void setCheckSkipOnBuildConst(bool value);

	signals:
		void projectUpdated(); // Project was loaded or cleared

		void scriptStarted();
		void scriptFinished();

	public:
		[[nodiscard]] ILogFile* log();

		[[nodiscard]] bool isLoaded() const;
		[[nodiscard]] QString buildPath() const;
		[[nodiscard]] int buildNo() const;

		[[nodiscard]] QString projectName() const;

		[[nodiscard]] const Sim::Connections& connections() const;
		[[nodiscard]] Sim::Connections& connections();

		[[nodiscard]] std::vector<Sim::Subsystem> subsystems() const;

		[[nodiscard]] std::optional<Sim::LogicModule> logicModule(QString equipmentId) const;
		[[nodiscard]] std::vector<Sim::LogicModule> logicModules() const;

		[[nodiscard]] Sim::AppSignalManager& appSignalManager();
		[[nodiscard]] const Sim::AppSignalManager& appSignalManager() const;

		[[nodiscard]] ITuningSignalManager& tuningSignalManagerInterface();
		[[nodiscard]] const ITuningSignalManager& tuningSignalManagerInterface() const;

		[[nodiscard]] Sim::OverrideSignals& overrideSignals();
		[[nodiscard]] const Sim::OverrideSignals& overrideSignals() const;

		[[nodiscard]] Sim::Software& software();
		[[nodiscard]] const Sim::Software& software() const;

		[[nodiscard]] Sim::Profiles& profiles();
		[[nodiscard]] const Sim::Profiles& profiles() const;

		bool setCurrentProfile(QString profileName);
		[[nodiscard]] QString currentProfileName() const;
		[[nodiscard]] const Sim::Profile& currentProfile() const;

		[[nodiscard]] Sim::Control& control();
		[[nodiscard]] const Sim::Control& control() const;

	private:
		std::unique_ptr<Sim::SimulatorPrivate> m_impl;
	};
} // namespace Sim
