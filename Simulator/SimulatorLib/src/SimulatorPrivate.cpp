#include "SimulatorPrivate.h"
#include "SimScopedLog.h"
#include "SimScriptDevUtils.h"
#include "SimScriptLogicModule.h"
#include "SimScriptRamAddress.h"
#include "SimScriptSignal.h"

#include <HardwareLib/LogicModulesInfo.h>
#include <HardwareLib/ModuleFirmware.h>

namespace Sim
{
	const QString SimulatorPrivate::DefaultProfileName = "Default";


	//
	// Simulator
	//
	SimulatorPrivate::SimulatorPrivate(ILogFile* log, bool allowDebugMessages, QObject* parent) :
		QObject{parent},
		m_log{log, allowDebugMessages, nullptr},
		m_tuningSignalManager{ScopedLog{log, allowDebugMessages, nullptr}},
		m_software{this},
		m_scriptSimulator{this}
	{
		qRegisterMetaType<AppSignalParam>("AppSignalParam");

		qRegisterMetaType<Sim::SimControlState>("SimControlState");
		qRegisterMetaType<Sim::ControlStatus>("ControlStatus");
		qRegisterMetaType<Sim::CyclePhase>("CyclePhase");
		// qRegisterMetaType<Sim::DeviceState>("DeviceState");

		qRegisterMetaType<Sim::RamAddress>("RamAddress");
		qRegisterMetaType<Sim::ScriptSignal>("ScriptSignal");
		qRegisterMetaType<Sim::ScriptLmDescription>("ScriptLmDescription");
		qRegisterMetaType<Sim::ScriptLogicModule>("ScriptLogicModule");
		qRegisterMetaType<Sim::ScriptDevUtils>("ScriptDevUtils");
		qRegisterMetaType<E::LogicModuleRamAccess>("LogicModuleRamAccess");

		return;
	}

	SimulatorPrivate::~SimulatorPrivate()
	{
		return;
	}

	bool SimulatorPrivate::load(QString buildPath)
	{
		clear(); // Clear must be run in this thread

		// Run load in separated thread, it'll allow to process messages, like timer events
		// for displaying output log
		//
		QFuture<bool> future = QtConcurrent::run(
			[buildPath, this]() -> bool
			{
				return this->loadFunc(buildPath);
			});

		while (future.isRunning() == true)
		{
			QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
			QThread::yieldCurrentThread();
		}

		bool result = future.result();

		if (result == true)
		{
			std::lock_guard locker{m_buildPathMutex};
			m_buildPath = buildPath;
		}
		else
		{
			clearImpl();
		}

		if (result == true)
		{
			// Should be run in this thread.
			//
			m_service.setEnabled(m_software.enabled());
		}

		emit projectUpdated();
		return result;
	}

	void SimulatorPrivate::clear()
	{
		clearImpl();
		emit projectUpdated();
		return;
	}

	bool SimulatorPrivate::isRunning() const
	{
		return m_controlImpl.state() == SimControlState::Run;
	}

	bool SimulatorPrivate::isPaused() const
	{
		return m_controlImpl.state() == SimControlState::Pause;
	}

	bool SimulatorPrivate::isStopped() const
	{
		return m_controlImpl.state() == SimControlState::Stop;
	}

	bool SimulatorPrivate::runScript(const SimScriptItem& script, const SimScriptItem& globalScript, qint64 timeout)
	{
		return runScripts({script}, globalScript, timeout);
	}

	bool SimulatorPrivate::runScripts(const std::vector<SimScriptItem>& scripts, const SimScriptItem& globalScript, qint64 timeout)
	{
		if (m_scriptSimulator.isRunning() == true)
		{
			m_scriptSimulator.stopScript();
		}

		m_scriptSimulator.log().setDebugMessagesEnabled(false);
		m_scriptSimulator.setExecutionTimeout(timeout);

		return m_scriptSimulator.runScripts(scripts, globalScript);
	}

	bool SimulatorPrivate::stopScript()
	{
		return m_scriptSimulator.stopScript();
	}

	bool SimulatorPrivate::waitScript(unsigned long msecs /*= ULONG_MAX*/)
	{
		return m_scriptSimulator.wait(msecs);
	}

	bool SimulatorPrivate::scriptResult()
	{
		return m_scriptSimulator.result();
	}

	bool SimulatorPrivate::checkSkipOnBuildConst() const
	{
		return m_scriptSimulator.checkSkipOnBuildConst();
	}

	void SimulatorPrivate::setCheckSkipOnBuildConst(bool value)
	{
		m_scriptSimulator.setCheckSkipOnBuildConst(value);
	}

	void SimulatorPrivate::clearImpl()
	{
		// Stops simulation if any.
		//
		m_controlImpl.reset();

		{
			// Empty m_buildPath indicates that project is not loaded
			//
			std::lock_guard locker{m_buildPathMutex};
			m_buildPath.clear();
		}

		// Stop service, so it will not access not protected members during project load.
		//
		m_service.setEnabled(false);

		m_firmwares.clear();
		m_lmDescriptions.clear();
		m_subsystems.clear();
		m_appSignalManager.resetAll();
		m_connectionsImpl.clear();
		m_software.clear();
		m_profiles.clear();
		m_currentProfileName = DefaultProfileName;

		return;
	}

	bool SimulatorPrivate::loadFunc(QString buildPath)
	{
		buildPath = QDir::fromNativeSeparators(buildPath);
		if (buildPath.endsWith(QChar('/')) == false)
		{
			buildPath.append(QChar('/'));
		}

		m_log.writeMessage(QLatin1String("Load project for simulation from ") + buildPath);

		//--
		//
		if (QFileInfo::exists(buildPath) == false)
		{
			m_log.writeError(QObject::tr("BuildPath %1 does not exist").arg(buildPath));
			return false;
		}

		// Load Software - file /Common/Software.xml
		//
		{
			bool ok = m_software.load(buildPath);

			if (ok == false)
			{
				return false;
			}
		}

		// Load simulator profiles
		//
		{
			QString profilesFileName = buildPath + Directory::COMMON + "/" + File::SIM_PROFILES;
			QFile file(profilesFileName);

			if (file.exists() == true)
			{
				bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);

				if (ok == false)
				{
					m_log.writeError(QObject::tr("Open simulator profiles file error. File %1").arg(profilesFileName));
					return false;
				}

				QString errorMessage;
				ok = m_profiles.load(file.readAll(), &errorMessage);

				if (ok == false)
				{
					m_log.writeError(
						QObject::tr("Load simulator profiles file error. File %1. Error %2").arg(profilesFileName).arg(errorMessage));
					return false;
				}
			}
			else
			{
				// It's ok if the file not exists
				//
			}
		}

		// Load bts file
		//
		bool ok = loadFirmwares(buildPath);
		if (ok == false)
		{
			return false;
		}

		QStringList subsystems = m_firmwares.subsystems();
		if (subsystems.isEmpty() == true)
		{
			m_log.writeWarning(QObject::tr("Bitstream file does not contain any subsystem."));
			m_log.writeWarning(QObject::tr("Nothing to load or simulate."));
			return true; // Project is empty, is not an error
		}

		// Load LogicModules Descriptions
		//
		ok = loadLmDescriptions(buildPath);
		if (ok == false)
		{
			return false;
		}

		// Load LogicModules info - file /Common/LogicModules.xml
		//
		LogicModulesInfo logicModulesInfo;

		{
			QString loadLmsInfoErrorMessage;
			QString lmsInfoFileName = buildPath + QString(Directory::COMMON) + "/" + QString(File::LOGIC_MODULES_XML);

			ok = logicModulesInfo.load(lmsInfoFileName, &loadLmsInfoErrorMessage);
			if (ok == false)
			{
				m_log.writeError(tr("Load file %1 error: %2").arg(lmsInfoFileName).arg(loadLmsInfoErrorMessage));
				return false;
			}
		}

		// Load ConnectionsInfo
		//
		ok = loadConnectionsInfo(buildPath);
		if (ok == false)
		{
			return false;
		}

		// Load subsystems
		//
		for (QString subsystemId : subsystems)
		{
			m_log.writeMessage(QObject::tr("Load subsystem: %1").arg(subsystemId));

			const Hardware::ModuleFirmware& firmware = m_firmwares.firmware(subsystemId, &ok);
			if (ok == false)
			{
				m_log.writeError(QObject::tr("Subsystem %1 in not found in bitstream file.").arg(subsystemId));
				return false;
			}

			// There are cases when subsystem does not have some UARTs (like BVB), then we cannot simulate such
			// subsystems, but still want to simulate other LM's
			//
			if (firmware.uartExists(static_cast<int>(UartId::ApplicationLogic)) == false)
			{
				m_log.writeWarning(QObject::tr("Subsystem %1 has no ApplicationLogic, it will not be simulated.").arg(subsystemId));
				continue;
			}

			if (firmware.uartExists(static_cast<int>(UartId::Tuning)) == false)
			{
				m_log.writeWarning(QObject::tr("Subsystem %1 has no Tuning, it will not be simulated.").arg(subsystemId));
				continue;
			}

			if (firmware.uartExists(static_cast<int>(UartId::Configuration)) == false)
			{
				m_log.writeWarning(QObject::tr("Subsystem %1 has no Configuration, it will not be simulated.").arg(subsystemId));
				continue;
			}

			// Create subsystem
			//
			if (m_subsystems.count(subsystemId) > 0)
			{
				m_log.writeError(QObject::tr("Subsystem %1 already exists.").arg(subsystemId));
				return false;
			}

			auto subsystem = std::make_shared<Sim::SubsystemImpl>(subsystemId, this);
			m_subsystems[subsystemId] = subsystem;

			SubsystemImpl& ss = *subsystem.get();

			// Get LogicModuleDescription
			//
			QString lmDescriptionFile = firmware.lmDescriptionFile();

			auto lmit = m_lmDescriptions.find(lmDescriptionFile);
			if (lmit == m_lmDescriptions.end())
			{
				m_log.writeError(QObject::tr("Cannot find LogicModule description file %1").arg(lmDescriptionFile));
				return false;
			}

			const LmDescription& lmDescription = *(lmit->second.get());

			// Upload data to subsystem
			//
			ok = ss.load(firmware, lmDescription, m_connectionsImpl, logicModulesInfo);
			if (ok == false)
			{
				// Error must be reported in Subsystem::load
				//
				return false;
			}
		}

		// Load application signals
		//
		ok = loadAppSignals(buildPath);
		if (ok == false)
		{
			return false;
		}

		// Update overridden signals
		//
		overrideSignals().updateSignals();

		// --
		//
		m_log.writeMessage("Project for simulation successfully loaded.");
		return true;
	}

	bool SimulatorPrivate::loadFirmwares(QString buildPath)
	{
		m_firmwares.clear();

		QDir dir(buildPath);
		if (dir.exists() == false)
		{
			m_log.writeError(QObject::tr("BuildPath %1 does not exist").arg(buildPath));
			return false;
		}

		QStringList btsFilter = {"*.bts"};
		QFileInfoList btsFiles = dir.entryInfoList(btsFilter, QDir::Files);

		if (btsFiles.size() == 0)
		{
			m_log.writeError(QObject::tr("Bitstream file not found, path %1").arg(buildPath));
			return false;
		}

		if (btsFiles.size() > 1)
		{
			m_log.writeError(QObject::tr("There are more than one bitstream file, path %1").arg(buildPath));
			return false;
		}

		QString btsFileName = btsFiles.front().canonicalFilePath();
		m_log.writeMessage(QObject::tr("Load bitstream file: %1").arg(btsFiles.front().fileName()));

		QString errorMessage;

		bool ok = m_firmwares.load(btsFileName, &errorMessage);
		if (ok == false)
		{
			m_log.writeError(QObject::tr("Loading bitstream file error: %1").arg(errorMessage));
			return false;
		}

		return true;
	}

	bool SimulatorPrivate::loadLmDescriptions(QString buildPath)
	{
		m_lmDescriptions.clear();

		QDir dir(buildPath);
		if (dir.exists() == false)
		{
			m_log.writeError(QObject::tr("BuildPath %1 does not exist").arg(buildPath));
			return false;
		}

		if (bool ok = dir.cd("LmDescriptions"); ok == false)
		{
			m_log.writeError(QObject::tr("Path %1/LmDescriptions does not exist").arg(buildPath));
			return false;
		}

		QStringList xmlFilter = {"*.xml"};
		QFileInfoList xmlFiles = dir.entryInfoList(xmlFilter, QDir::Files);

		if (xmlFiles.size() == 0)
		{
			m_log.writeError(QObject::tr("LogicModule description file(s) not found, path %1").arg(buildPath));
			return false;
		}

		for (QFileInfo& fi : xmlFiles)
		{
			QString fileName = fi.canonicalFilePath();
			m_log.writeMessage(QObject::tr("Load LogicModule description file: %1").arg(fi.fileName()));

			QFile file(fileName);

			if (bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text); ok == false)
			{
				m_log.writeError(QObject::tr("Open file error: %1").arg(file.errorString()));
				return false;
			}

			QByteArray xmlData = file.readAll();

			QString errorMessage;
			std::shared_ptr<LmDescription> lmDescription = std::make_shared<LmDescription>();

			if (bool ok = lmDescription->load(xmlData, &errorMessage); ok == false)
			{
				m_log.writeError(QObject::tr("Loading file %1 error: %2").arg(fileName).arg(errorMessage));
				return false;
			}

			m_lmDescriptions[fi.fileName()] = lmDescription;
		}

		return true;
	}

	bool SimulatorPrivate::loadConnectionsInfo(QString buildPath)
	{
		QString fileName = QDir::fromNativeSeparators(buildPath);
		if (fileName.endsWith(QChar('/')) == false)
		{
			fileName.append(QChar('/'));
		}

		fileName += QString(Directory::COMMON) + "/" + QString(File::CONNECTIONS_XML);

		m_log.writeMessage(tr("Loading %1").arg(fileName));

		QString errorMessage;
		bool ok = m_connectionsImpl.load(fileName, &errorMessage);
		if (ok == false)
		{
			m_log.writeError(tr("File loading error, file name %1, error:%2").arg(fileName).arg(errorMessage));
		}

		return ok;
	}

	bool SimulatorPrivate::loadAppSignals(QString buildPath)
	{
		QString fileName = QDir::fromNativeSeparators(buildPath);
		if (fileName.endsWith(QChar('/')) == false)
		{
			fileName.append(QChar('/'));
		}

		fileName += QString(Directory::COMMON) + "/" + QString(File::APP_SIGNALS_ASGS);

		m_log.writeMessage(tr("Loading %1").arg(fileName));

		bool ok = m_appSignalManager.load(fileName);
		if (ok == false)
		{
			m_log.writeError(tr("File loading error, file name %1.").arg(fileName));
		}

		return ok;
	}

	ScopedLog& SimulatorPrivate::log()
	{
		return m_log;
	}

	bool SimulatorPrivate::isLoaded() const
	{
		std::lock_guard locker{m_buildPathMutex};
		return m_buildPath.isEmpty() == false;
	}

	QString SimulatorPrivate::buildPath() const
	{
		std::lock_guard locker{m_buildPathMutex};
		return m_buildPath;
	}

	int SimulatorPrivate::buildNo() const
	{
		return m_firmwares.buildNumber();
	}

	QString SimulatorPrivate::projectName() const
	{
		return m_firmwares.projectName();
	}

	const Sim::ConnectionsImpl& SimulatorPrivate::connections() const
	{
		return m_connectionsImpl;
	}

	Sim::ConnectionsImpl& SimulatorPrivate::connections()
	{
		return m_connectionsImpl;
	}

	const Sim::Connections& SimulatorPrivate::connectionsPublic() const
	{
		return m_connectionsPublic;
	}

	Sim::Connections& SimulatorPrivate::connectionsPublic()
	{
		return m_connectionsPublic;
	}

	std::vector<std::shared_ptr<SubsystemImpl>> SimulatorPrivate::subsystems() const
	{
		std::vector<std::shared_ptr<SubsystemImpl>> result;
		result.reserve(m_subsystems.size());

		for (const auto& [key, ss] : m_subsystems)
		{
			Q_UNUSED(key)
			result.push_back(ss);
		}

		return result;
	}

	std::shared_ptr<LogicModuleImpl> SimulatorPrivate::logicModule(QString equipmentId) const
	{
		for (const auto& [key, ss] : m_subsystems)
		{
			Q_UNUSED(key);

			std::shared_ptr<LogicModuleImpl> lm = ss->logicModule(equipmentId);
			if (lm != nullptr)
			{
				return lm;
			}
		}

		return {};
	}

	std::vector<std::shared_ptr<LogicModuleImpl>> SimulatorPrivate::logicModules() const
	{
		std::vector<std::shared_ptr<LogicModuleImpl>> result;
		result.reserve(m_subsystems.size() * 10); // Just some number

		for (const auto& [key, ss] : m_subsystems)
		{
			Q_UNUSED(key);

			std::vector<std::shared_ptr<LogicModuleImpl>> subsystemModules = ss->logicModules();
			result.insert(result.end(), subsystemModules.begin(), subsystemModules.end());
		}

		return result;
	}

	Sim::AppSignalManagerImpl& SimulatorPrivate::appSignalManager()
	{
		return m_appSignalManager;
	}

	const Sim::AppSignalManagerImpl& SimulatorPrivate::appSignalManager() const
	{
		return m_appSignalManager;
	}

	Sim::AppSignalManager& SimulatorPrivate::appSignalManagerPublic()
	{
		return m_appSignalManagerPublic;
	}

	const Sim::AppSignalManager& SimulatorPrivate::appSignalManagerPublic() const
	{
		return m_appSignalManagerPublic;
	}

	Sim::TuningSignalManager& SimulatorPrivate::tuningSignalManager()
	{
		return m_tuningSignalManager;
	}

	const Sim::TuningSignalManager& SimulatorPrivate::tuningSignalManager() const
	{
		return m_tuningSignalManager;
	}

	Sim::OverrideSignalsImpl& SimulatorPrivate::overrideSignals()
	{
		return m_overrideSignals;
	}

	const Sim::OverrideSignalsImpl& SimulatorPrivate::overrideSignals() const
	{
		return m_overrideSignals;
	}

	Sim::OverrideSignals& SimulatorPrivate::overrideSignalsPublic()
	{
		return m_overrideSignalsPublic;
	}

	const Sim::OverrideSignals& SimulatorPrivate::overrideSignalsPublic() const
	{
		return m_overrideSignalsPublic;
	}

	Sim::SoftwareImpl& SimulatorPrivate::software()
	{
		return m_software;
	}

	const Sim::SoftwareImpl& SimulatorPrivate::software() const
	{
		return m_software;
	}

	Sim::Software& SimulatorPrivate::softwarePublic()
	{
		return m_softwarePublic;
	}

	const Sim::Software& SimulatorPrivate::softwarePublic() const
	{
		return m_softwarePublic;
	}

	Sim::Profiles& SimulatorPrivate::profiles()
	{
		return m_profiles;
	}

	const Sim::Profiles& SimulatorPrivate::profiles() const
	{
		return m_profiles;
	}

	Sim::Service& SimulatorPrivate::service()
	{
		return m_service;
	}

	const Sim::Service& SimulatorPrivate::service() const
	{
		return m_service;
	}

	bool SimulatorPrivate::setCurrentProfile(QString profileName)
	{
		if (profileName.isEmpty() == true)
		{
			profileName = DefaultProfileName;
		}

		if (profiles().hasProfile(profileName) == false)
		{
			m_log.writeError(tr("Cannot set profile %1, this profile not found").arg(profileName));

			m_currentProfileName = DefaultProfileName;
			return false;
		}

		m_currentProfileName = profileName;
		return true;
	}

	QString SimulatorPrivate::currentProfileName() const
	{
		return m_currentProfileName;
	}

	const Sim::Profile& SimulatorPrivate::currentProfile() const
	{
		return m_profiles.profile(m_currentProfileName);
	}

	Sim::ControlImpl& SimulatorPrivate::control()
	{
		return m_controlImpl;
	}

	const Sim::ControlImpl& SimulatorPrivate::control() const
	{
		return m_controlImpl;
	}

	Sim::Control& SimulatorPrivate::controlPublic()
	{
		return m_controlPublic;
	}

	const Sim::Control& SimulatorPrivate::controlPublic() const
	{
		return m_controlPublic;
	}

} // namespace Sim
