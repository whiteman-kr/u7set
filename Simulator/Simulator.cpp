#include "Simulator.h"
#include <cassert>
#include <QDebug>
#include <QDir>
#include "../lib/ModuleFirmware.h"

namespace Sim
{
	//
	// Simulator
	//
	Simulator::Simulator(QTextStream* outputStream) :
		Output(outputStream)
	{
		return;
	}

	Simulator::~Simulator()
	{
	}

	bool Simulator::load(QString buildPath)
	{
		m_subsystems.clear();

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
			return false;
		}

		QStringList subsystems = m_firmwares.subsystems();
		if (subsystems.isEmpty() == true)
		{
			writeError(QObject::tr("Bitstream file does not contain any subsystem."));
			return false;
		}

		// Load LoficModules Descriptions
		//
		ok = loadLmDescriptions(buildPath);
		if (ok == false)
		{
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
			}

			// Create subsystem
			//
			const Output& out = *this;
			auto res = m_subsystems.try_emplace(subsystemId, subsystemId, out);

			if (res.second == false)
			{
				writeError(QObject::tr("Subsystem %1 already exists.").arg(subsystemId));
				return false;
			}

			auto it = res.first;
			Subsystem& ss = it->second;

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

			// Upload data to susbystem
			//
			ok = ss.load(firmware, lmDescription);
			if (ok == false)
			{
				// Error must be reported in Subsystem::load
				//
				return false;
			}

		}

		// Load LmDescriptions
		//

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

}
