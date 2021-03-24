#include "Simulator.h"
#include <cassert>
#include <QDebug>
#include <QDir>
#include <QtConcurrent/QtConcurrent>
#include "../lib/ModuleFirmware.h"
#include "../lib/LogicModulesInfo.h"
#include "SimScriptRamAddress.h"
#include "SimScriptLogicModule.h"
#include "SimScriptSignal.h"
#include "SimScriptDevUtils.h"
#include "SimScopedLog.h"


namespace Sim
{
	const QString Simulator::DefaultProfileName = "Default";


	//
	// Simulator
	//
	Simulator::Simulator(ILogFile* log, QObject* parent) :
		QObject{parent},
		m_log{log, {}},
		m_tuningSignalManager{ScopedLog{log, {}}},
		m_software{this},
		m_scriptSimulator{this}
	{
		qRegisterMetaType<AppSignalParam>("AppSignalParam");

		qRegisterMetaType<Sim::SimControlState>("SimControlState");
		qRegisterMetaType<Sim::ControlStatus>("ControlStatus");
		qRegisterMetaType<Sim::CyclePhase>("CyclePhase");
		qRegisterMetaType<Sim::DeviceState>("DeviceState");

		qRegisterMetaType<Sim::RamAddress>("RamAddress");
		qRegisterMetaType<Sim::ScriptSignal>("ScriptSignal");
		qRegisterMetaType<Sim::ScriptLmDescription>("ScriptLmDescription");
		qRegisterMetaType<Sim::ScriptLogicModule>("ScriptLogicModule");
		qRegisterMetaType<Sim::ScriptDevUtils>("ScriptDevUtils");
		qRegisterMetaType<E::LogicModuleRamAccess>("LogicModuleRamAccess");

		return;
	}

	Simulator::~Simulator()
	{
		return;
	}

	bool Simulator::load(QString buildPath)
	{
		clear();	// Clear must be run in this thread

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
			m_buildPath = buildPath;
		}
		else
		{
			clearImpl();
		}

		emit projectUpdated();
		return result;
	}

	void Simulator::clear()
	{
		clearImpl();
		emit projectUpdated();
		return;
	}

	bool Simulator::isRunning() const
	{
		return m_control.state() == SimControlState::Run;
	}

	bool Simulator::isPaused() const
	{
		return m_control.state() == SimControlState::Pause;
	}

	bool Simulator::isStopped() const
	{
		return m_control.state() == SimControlState::Stop;
	}

	bool Simulator::runScript(const SimScriptItem& script, qint64 timeout)
	{
		return runScripts({script}, timeout);
	}

	bool Simulator::runScripts(const std::vector<SimScriptItem>& scripts, qint64 timeout)
	{
		if (m_scriptSimulator.isRunning() == true)
		{
			m_scriptSimulator.stopScript();
		}

		m_scriptSimulator.setExecutionTimeout(timeout);

		return m_scriptSimulator.runScripts(scripts);
	}

	bool Simulator::stopScript()
	{
		return m_scriptSimulator.stopScript();
	}

	bool Simulator::waitScript(unsigned long msecs /*= ULONG_MAX*/)
	{
		return m_scriptSimulator.wait(msecs);
	}

	bool Simulator::scriptResult()
	{
		return m_scriptSimulator.result();
	}

	void Simulator::clearImpl()
	{
		m_control.reset();
		m_buildPath.clear();
		m_firmwares.clear();
		m_lmDescriptions.clear();
		m_subsystems.clear();
		m_appSignalManager.resetAll();
		m_connections.clear();
		m_software.clear();
		m_profiles.clear();
		m_currentProfileName = DefaultProfileName;

		return;
	}

	bool Simulator::loadFunc(QString buildPath)
	{
		clearImpl();

		// --
		//
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
			QString softwareFileName = buildPath + Directory::COMMON + "/" + File::SOFTWARE_XML;
			bool ok = m_software.loadSoftwareXml(softwareFileName);

			if (ok == false)
			{
				m_log.writeError(QObject::tr("Load sofware description error, file %1 not found or corrupted").arg(softwareFileName));

				clearImpl();
				return false;
			}
		}

		// Load simulator profiles
		//
		{
			QString profilesFileName = buildPath + Directory::COMMON + "/" + Db::File::SimProfilesFileName;
			QFile file(profilesFileName);

			if (file.exists() == true)
			{
				bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);

				if (ok == false)
				{
					m_log.writeError(QObject::tr("Open simulator profiles file error. File %1").arg(profilesFileName));
					clearImpl();
					return false;
				}

				QString errorMessage;
				ok = m_profiles.load(file.readAll(), &errorMessage);

				if (ok == false)
				{
					m_log.writeError(QObject::tr("Load simulator profiles file error. File %1. Error %2")
									 .arg(profilesFileName)
									 .arg(errorMessage));
					clearImpl();
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
			clearImpl();
			return false;
		}

		QStringList subsystems = m_firmwares.subsystems();
		if (subsystems.isEmpty() == true)
		{
			m_log.writeWarning(QObject::tr("Bitstream file does not contain any subsystem."));
			m_log.writeWarning(QObject::tr("Nothing to load or simulate."));
			clearImpl();
			return true;	// Project is empty, is not an error
		}

		// Load LogicModules Descriptions
		//
		ok = loadLmDescriptions(buildPath);
		if (ok == false)
		{
			clearImpl();
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
				clearImpl();
				return false;
			}
		}

		// Load ConnectinsInfo
		//
		ok = loadConnectionsInfo(buildPath);
		if (ok == false)
		{
			clearImpl();
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
				clearImpl();
				return false;
			}

			// There are cases when subsustem does not have some UARTs (like BVB), then we cannot simulate such
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
				m_log.writeWarning(QObject::tr("Subsystem %1 has no Congiguration, it will not be simulated.").arg(subsystemId));
				continue;
			}

			// Create subsystem
			//
			if (m_subsystems.count(subsystemId) > 0)
			{
				m_log.writeError(QObject::tr("Subsystem %1 already exists.").arg(subsystemId));
				clearImpl();
				return false;
			}

			auto subsystem = std::make_shared<Sim::Subsystem>(subsystemId, this);
			m_subsystems[subsystemId] = subsystem;

			Subsystem& ss = *subsystem.get();

			// Get LogicMoudelDescription
			//
			QString lmDescriptionFile = firmware.lmDescriptionFile();

			auto lmit = m_lmDescriptions.find(lmDescriptionFile);
			if (lmit == m_lmDescriptions.end())
			{
				m_log.writeError(QObject::tr("Cannot find LogicModule description file %1").arg(lmDescriptionFile));
				return false;
			}

			const LmDescription& lmDescription = *(lmit->second.get());

			// Upload data to susbystem
			//
			ok = ss.load(firmware, lmDescription, m_connections, logicModulesInfo);
			if (ok == false)
			{
				// Error must be reported in Subsystem::load
				//
				clearImpl();
				return false;
			}
		}

		// Load appilcation signals
		//
		ok = loadAppSignals(buildPath);
		if (ok == false)
		{
			clearImpl();
			return false;
		}

		// Update overriden signals
		//
		overrideSignals().updateSignals();

		// --
		//
		m_log.writeMessage("Project for simulation successfully loaded.");
		return true;
	}

	bool Simulator::loadFirmwares(QString buildPath)
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

	bool Simulator::loadLmDescriptions(QString buildPath)
	{
		m_lmDescriptions.clear();

		QDir dir(buildPath);
		if (dir.exists() == false)
		{
			m_log.writeError(QObject::tr("BuildPath %1 does not exist").arg(buildPath));
			return false;
		}

		if (bool ok = dir.cd("LmDescriptions");
			ok == false)
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

			if (bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
				ok == false)
			{
				m_log.writeError(QObject::tr("Open file error: %1")
								  .arg(file.errorString()));
				return false;
			}

			QByteArray xmlData = file.readAll();

			QString errorMessage;
			std::shared_ptr<LmDescription> lmDescription = std::make_shared<LmDescription>();

			if (bool ok = lmDescription->load(xmlData, &errorMessage);
				ok == false)
			{
				m_log.writeError(QObject::tr("Loading file %1 error: %2")
								  .arg(fileName)
								  .arg(errorMessage));
				return false;
			}

			m_lmDescriptions[fi.fileName()] = lmDescription;
		}

		return true;
	}

	bool Simulator::loadConnectionsInfo(QString buildPath)
	{
		QString fileName = QDir::fromNativeSeparators(buildPath);
		if (fileName.endsWith(QChar('/')) == false)
		{
			fileName.append(QChar('/'));
		}

		fileName += QString(Directory::COMMON) + "/" + QString(File::CONNECTIONS_XML);

		m_log.writeMessage(tr("Loading %1").arg(fileName));

		QString errorMessage;
		bool ok = m_connections.load(fileName, &errorMessage);
		if (ok == false)
		{
			m_log.writeError(tr("File loading error, file name %1, error:%2").arg(fileName).arg(errorMessage));
		}

		return ok;
	}

	bool Simulator::loadAppSignals(QString buildPath)
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

	ScopedLog& Simulator::log()
	{
		return m_log;
	}

	bool Simulator::isLoaded() const
	{
		return m_buildPath.isEmpty() == false;
	}

	QString Simulator::buildPath() const
	{
		return m_buildPath;
	}

	QString Simulator::projectName() const
	{
		return m_firmwares.projectName();
	}

	const Sim::Connections& Simulator::connections() const
	{
		return m_connections;
	}

	Sim::Connections& Simulator::connections()
	{
		return m_connections;
	}

	std::vector<std::shared_ptr<Subsystem>> Simulator::subsystems() const
	{
		std::vector<std::shared_ptr<Subsystem>> result;
		result.reserve(m_subsystems.size());

		for (const auto&[key, ss] : m_subsystems)
		{
			Q_UNUSED(key)
			result.push_back(ss);
		}

		return result;
	}

	std::shared_ptr<LogicModule> Simulator::logicModule(QString equipmentId) const
	{
		for (const auto&[key, ss] : m_subsystems)
		{
			Q_UNUSED(key);

			std::shared_ptr<LogicModule> lm = ss->logicModule(equipmentId);
			if (lm != nullptr)
			{
				return lm;
			}
		}

		return std::shared_ptr<LogicModule>();
	}

	std::vector<std::shared_ptr<LogicModule>> Simulator::logicModules() const
	{
		std::vector<std::shared_ptr<LogicModule>> result;
		result.reserve(m_subsystems.size() * 10);			// Just some number

		for (const auto&[key, ss] : m_subsystems)
		{
			Q_UNUSED(key);

			std::vector<std::shared_ptr<LogicModule>> subsystemModules = ss->logicModules();
			result.insert(result.end(), subsystemModules.begin(), subsystemModules.end());
		}

		return result;
	}

	Sim::AppSignalManager& Simulator::appSignalManager()
	{
		return m_appSignalManager;
	}

	const Sim::AppSignalManager& Simulator::appSignalManager() const
	{
		return m_appSignalManager;
	}

	Sim::TuningSignalManager& Simulator::tuningSignalManager()
	{
		return m_tuningSignalManager;
	}

	const Sim::TuningSignalManager& Simulator::tuningSignalManager() const
	{
		return m_tuningSignalManager;
	}

	Sim::OverrideSignals& Simulator::overrideSignals()
	{
		return m_overrideSignals;
	}

	const Sim::OverrideSignals& Simulator::overrideSignals() const
	{
		return m_overrideSignals;
	}

	Sim::Software& Simulator::software()
	{
		return m_software;
	}

	const Sim::Software& Simulator::software() const
	{
		return m_software;
	}

	Sim::Profiles& Simulator::profiles()
	{
		return m_profiles;
	}

	const Sim::Profiles& Simulator::profiles() const
	{
		return m_profiles;
	}

	bool Simulator::setCurrentProfile(QString profileName)
	{
		if (profiles().hasProfile(profileName) == false)
		{
			m_log.writeError(tr("Cannot set profile %1, this profile not found").arg(profileName));

			m_currentProfileName = DefaultProfileName;
			return false;
		}

		m_currentProfileName = profileName;
		return true;
	}

	QString Simulator::currentProfileName() const
	{
		return m_currentProfileName;
	}

	const Sim::Profile& Simulator::currentProfile() const
	{
		return m_profiles.profile(m_currentProfileName);
	}

	Sim::Control& Simulator::control()
	{
		return m_control;
	}

	const Sim::Control& Simulator::control() const
	{
		return m_control;
	}

}
