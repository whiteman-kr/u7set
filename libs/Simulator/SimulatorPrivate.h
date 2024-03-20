#pragma once

#include "../UtilsLib/ILogFile.h"
#include <HardwareLib/LmDescription.h>
#include <HardwareLib/ModuleFirmware.h>

#include "./include/Simulator/SimAppSignalManager.h"
#include "./include/Simulator/SimConnections.h"
#include "./include/Simulator/SimControl.h"
#include "./include/Simulator/SimOverrideSignals.h"
#include "./include/Simulator/SimProfiles.h"
#include "./include/Simulator/SimSoftware.h"

#include "SimAppSignalManagerImpl.h"
#include "SimConnectionsImpl.h"
#include "SimControlImpl.h"
#include "SimOverrideSignalsImpl.h"
#include "SimScopedLog.h"
#include "SimScriptSimulator.h"
#include "SimSoftwareImpl.h"
#include "SimSubsystemImpl.h"
#include "SimTuningSignalManager.h"


namespace Sim
{
	class LogicModuleImpl;


	class SimulatorPrivate : public QObject
	{
		Q_OBJECT

	public:
		explicit SimulatorPrivate(ILogFile* log, bool allowDebugMessages, QObject* parent); // if log is nullptr then log to console
		virtual ~SimulatorPrivate();

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

	private:
		void clearImpl();
		bool loadFunc(QString buildPath);
		bool loadFirmwares(QString buildPath);
		bool loadLmDescriptions(QString buildPath);
		bool loadConnectionsInfo(QString buildPath);
		bool loadAppSignals(QString buildPath);

	signals:
		void projectUpdated(); // Project was loaded or cleared

		void scriptStarted();
		void scriptFinished();

	public:
		[[nodiscard]] ScopedLog& log();

		[[nodiscard]] bool isLoaded() const;
		[[nodiscard]] QString buildPath() const;
		[[nodiscard]] int buildNo() const;

		[[nodiscard]] QString projectName() const;

		[[nodiscard]] const Sim::ConnectionsImpl& connections() const;
		[[nodiscard]] Sim::ConnectionsImpl& connections();

		// The public interface of the connections.
		//
		[[nodiscard]] const Sim::Connections& connectionsPublic() const;
		[[nodiscard]] Sim::Connections& connectionsPublic();

		[[nodiscard]] std::vector<std::shared_ptr<SubsystemImpl>> subsystems() const;
		[[nodiscard]] std::shared_ptr<LogicModuleImpl> logicModule(QString equipmentId) const;
		[[nodiscard]] std::vector<std::shared_ptr<LogicModuleImpl>> logicModules() const;

		[[nodiscard]] Sim::AppSignalManagerImpl& appSignalManager();
		[[nodiscard]] const Sim::AppSignalManagerImpl& appSignalManager() const;

		[[nodiscard]] Sim::AppSignalManager& appSignalManagerPublic();
		[[nodiscard]] const Sim::AppSignalManager& appSignalManagerPublic() const;

		[[nodiscard]] Sim::TuningSignalManager& tuningSignalManager();
		[[nodiscard]] const Sim::TuningSignalManager& tuningSignalManager() const;

		[[nodiscard]] Sim::OverrideSignalsImpl& overrideSignals();
		[[nodiscard]] const Sim::OverrideSignalsImpl& overrideSignals() const;

		[[nodiscard]] Sim::OverrideSignals& overrideSignalsPublic();
		[[nodiscard]] const Sim::OverrideSignals& overrideSignalsPublic() const;

		[[nodiscard]] Sim::SoftwareImpl& software();
		[[nodiscard]] const Sim::SoftwareImpl& software() const;

		[[nodiscard]] Sim::Software& softwarePublic();				// Public API for Software
		[[nodiscard]] const Sim::Software& softwarePublic() const;	// Public API for Software

		[[nodiscard]] Sim::Profiles& profiles();
		[[nodiscard]] const Sim::Profiles& profiles() const;

		bool setCurrentProfile(QString profileName);
		[[nodiscard]] QString currentProfileName() const;
		[[nodiscard]] const Sim::Profile& currentProfile() const;

		[[nodiscard]] Sim::ControlImpl& control();
		[[nodiscard]] const Sim::ControlImpl& control() const;

		[[nodiscard]] Sim::Control& controlPublic();
		[[nodiscard]] const Sim::Control& controlPublic() const;

	private:
		mutable ScopedLog m_log;

		QString m_buildPath;
		Hardware::ModuleFirmwareStorage m_firmwares;                        // Loaded bts file

		Sim::ConnectionsImpl m_connectionsImpl;                             // Implementation of connections.
		Sim::Connections m_connectionsPublic{m_connectionsImpl, this};      // Connections, the public part, takes m_connectionsImpl.


		std::map<QString, std::shared_ptr<LmDescription>> m_lmDescriptions; // Key is filename
		std::map<QString, std::shared_ptr<SubsystemImpl>> m_subsystems;     // Key is SubsystemID

		// Signals Management
		//
		Sim::AppSignalManagerImpl m_appSignalManager{this};
		Sim::AppSignalManager m_appSignalManagerPublic{m_appSignalManager};

		Sim::TuningSignalManager m_tuningSignalManager;

		Sim::OverrideSignalsImpl m_overrideSignals{this};
		Sim::OverrideSignals m_overrideSignalsPublic{m_overrideSignals, &m_overrideSignals};

		// Software Info
		//
		Sim::SoftwareImpl m_software;
		Sim::Software m_softwarePublic{m_software};

		// Software profiles - different software settings can be applied via these profiles
		//
		static const QString DefaultProfileName;

		Sim::Profiles m_profiles;
		QString m_currentProfileName = DefaultProfileName;

		// Control thread
		//
		Sim::ControlImpl m_controlImpl{this};
		Sim::Control m_controlPublic{m_controlImpl, this};

		// Scripting/Testing
		//
		ScriptSimulator m_scriptSimulator;
	};
} // namespace Sim
