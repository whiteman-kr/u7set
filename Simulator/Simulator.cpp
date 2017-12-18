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

		QDir dir(buildPath);

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

			const Hardware::ModuleFirmware& firmware = mfs.firmware(subsystemId, &ok);
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

	bool SimulatorloadLmDescriptions(QString buildPath)
	{

	}

}
