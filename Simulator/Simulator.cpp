#include "Simulator.h"
#include <cassert>
#include <QDebug>
#include <QDir>
#include <QtConcurrent>
#include "../lib/ModuleFirmware.h"
#include "SimLmModel.h"

namespace Sim
{
	//
	// Simulator
	//
	Simulator::Simulator() :
		QObject(),
		Output()
	{
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
								[buildPath, this]()
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

	void Simulator::clearImpl()
	{
		m_buildPath.clear();
		m_firmwares.clear();
		m_lmDescriptions.clear();
		m_simScript.clear();
		m_subsystems.clear();

		return;
	}

	bool Simulator::loadFunc(QString buildPath)
	{
		writeMessage(QLatin1String("Load project for simulation from ") + buildPath);

		//--
		//
		if (QFileInfo::exists(buildPath) == false)
		{
			writeError(QObject::tr("BuildPath %1 does not exist").arg(buildPath));
			return false;
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
			writeError(QObject::tr("Bitstream file does not contain any subsystem."));
			clearImpl();
			return false;
		}

		// Load LogicModules Descriptions
		//
		ok = loadLmDescriptions(buildPath);
		if (ok == false)
		{
			clearImpl();
			return false;
		}

		// Load AFBL simulation scripts
		//
		ok = loadSimulationScripts(buildPath);
		if (ok == false)
		{
			clearImpl();
			return false;
		}

		// Load subsystems
		//
		for (QString subsystemId : subsystems)
		{
			writeMessage(QObject::tr("Load subsystem: %1").arg(subsystemId));

			const Hardware::ModuleFirmware& firmware = m_firmwares.firmware(subsystemId, &ok);
			if (ok == false)
			{
				writeError(QObject::tr("Subsystem %1 in not found in bitstream file.").arg(subsystemId));
				clearImpl();
				return false;
			}

			// Create subsystem
			//
			if (m_subsystems.count(subsystemId) > 0)
			{
				writeError(QObject::tr("Subsystem %1 already exists.").arg(subsystemId));
				clearImpl();
				return false;
			}

			auto subsystem = std::make_shared<Sim::Subsystem>(subsystemId);
			m_subsystems[subsystemId] = subsystem;

			Subsystem& ss = *subsystem.get();

			// Get LogicMoudelDescription
			//
			QString lmDescriptionFile = firmware.lmDescriptionFile();

			auto lmit = m_lmDescriptions.find(lmDescriptionFile);
			if (lmit == m_lmDescriptions.end())
			{
				writeError(QObject::tr("Cannot find LogicModule description file %1").arg(lmDescriptionFile));
				return false;
			}

			const LmDescription& lmDescription = *(lmit->second.get());

			// Get simulation script
			//
			auto simScriptIt = m_simScript.find(lmDescription.simualtionScriptFile());
			if (simScriptIt == m_simScript.end())
			{
				writeError(QObject::tr("Cannot find AFBL simulation script file %1").arg(lmDescription.simualtionScriptFile()));
				clearImpl();
				return false;
			}

			const QString& simulationScript = simScriptIt->second;

			// Upload data to susbystem
			//
			ok = ss.load(firmware, lmDescription, simulationScript);
			if (ok == false)
			{
				// Error must be reported in Subsystem::load
				//
				clearImpl();
				return false;
			}
		}

		writeMessage("Project for simulation successfully loaded.");
		return true;
	}

	bool Simulator::loadFirmwares(QString buildPath)
	{
		m_firmwares.clear();

		QDir dir(buildPath);
		if (dir.exists() == false)
		{
			writeError(QObject::tr("BuildPath %1 does not exist").arg(buildPath));
			return false;
		}

		QStringList btsFilter = {"*.bts"};
		QFileInfoList btsFiles = dir.entryInfoList(btsFilter, QDir::Files);

		if (btsFiles.size() == 0)
		{
			writeError(QObject::tr("Bitstream file not found, path %1").arg(buildPath));
			return false;
		}

		if (btsFiles.size() > 1)
		{
			writeError(QObject::tr("There are more than one bitstream file, path %1").arg(buildPath));
			return false;
		}

		QString btsFileName = btsFiles.front().canonicalFilePath();
		writeMessage(QObject::tr("Load bitstream file: %1").arg(btsFiles.front().fileName()));

		QString errorMessage;

		bool ok = m_firmwares.load(btsFileName, &errorMessage);
		if (ok == false)
		{
			writeError(QObject::tr("Loading bitstream file error: %1").arg(errorMessage));
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
			writeError(QObject::tr("BuildPath %1 does not exist").arg(buildPath));
			return false;
		}

		bool ok = dir.cd("LmDescriptions");
		if (ok == false)
		{
			writeError(QObject::tr("Path %1/LmDescriptions does not exist").arg(buildPath));
			return false;
		}

		QStringList xmlFilter = {"*.xml"};
		QFileInfoList xmlFiles = dir.entryInfoList(xmlFilter, QDir::Files);

		if (xmlFiles.size() == 0)
		{
			writeError(QObject::tr("LogicModule description file(s) not found, path %1").arg(buildPath));
			return false;
		}

		for (QFileInfo& fi : xmlFiles)
		{
			QString fileName = fi.canonicalFilePath();
			writeMessage(QObject::tr("Load LogicModule description file: %1").arg(fi.fileName()));

			QFile file(fileName);
			ok = file.open(QIODevice::ReadOnly | QIODevice::Text);

			if (ok == false)
			{
				writeError(QObject::tr("Open file error: %1")
							.arg(file.errorString()));
				return false;
			}

			QByteArray xmlData = file.readAll();

			QString errorMessage;
			std::shared_ptr<LmDescription> lmDescription = std::make_shared<LmDescription>();

			ok = lmDescription->load(xmlData, &errorMessage);
			if (ok == false)
			{
				writeError(QObject::tr("Loading file %1 error: %2")
							.arg(fileName)
							.arg(errorMessage));
				return false;
			}

			m_lmDescriptions[fi.fileName()] = lmDescription;
		}

		return true;
	}

	bool Simulator::loadSimulationScripts(QString buildPath)
	{
		m_simScript.clear();

		QDir dir(buildPath);
		if (dir.exists() == false)
		{
			writeError(QObject::tr("BuildPath %1 does not exist").arg(buildPath));
			return false;
		}

		bool ok = dir.cd("Simulation");
		if (ok == false)
		{
			writeError(QObject::tr("Path %1/Simulation does not exist").arg(buildPath));
			return false;
		}

		QStringList sjFilter = {"*.js"};
		QFileInfoList jsFiles = dir.entryInfoList(sjFilter, QDir::Files);

		for (QFileInfo& fi : jsFiles)
		{
			QString fileName = fi.canonicalFilePath();
			writeMessage(QObject::tr("Load AFBL simulation script: %1").arg(fi.fileName()));

			QFile file(fileName);
			ok = file.open(QIODevice::ReadOnly | QIODevice::Text);

			if (ok == false)
			{
				writeError(QObject::tr("Open file error: %1")
							.arg(file.errorString()));
				return false;
			}

			QByteArray jsData = file.readAll();
			QString js(jsData);

			m_simScript[fi.fileName()] = js;
		}

		return true;
	}

	bool Simulator::isLoaded() const
	{
		return m_buildPath.isEmpty() == false;
	}

	QString Simulator::buildPath() const
	{
		return m_buildPath;
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

	std::shared_ptr<LogicModule> Simulator::logicModule(QString equipmentId)
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

}
