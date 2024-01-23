#include "SimSoftware.h"
#include "Simulator.h"

namespace  Sim
{

	Software::Software(Simulator* simulator) :
		m_simulator(simulator),
		m_log(simulator->log(), "Software"),
		m_appDataTransmitter{simulator}
	{
		Q_ASSERT(m_simulator);
	}

	void Software::clear()
	{
		m_software.clear();
		m_tuningServiceCommunicators.clear();

		return;
	}

	bool Software::load(QString buildPath)
	{
		clear();

		bool ok = loadSoftwareXml(buildPath);

		return ok;
	}

	bool Software::loadSoftwareXml(QString buildPath)
	{
		clear();

		QString softwareFileName = buildPath + Directory::COMMON + "/" + File::SOFTWARE_XML;

		// Raad and parse file /Common/Software.xml
		//
		::SoftwareXmlReader reader;

		bool ok = reader.readSoftwareXml(softwareFileName);
		if (ok == false)
		{
			m_log.writeError(QObject::tr("Load sofware description error, file %1 not found or corrupted").arg(softwareFileName));
			clear();

			return false;
		}

		// --
		//
		const std::map<QString, SoftwareXmlInfo>& sxi = reader.softwareXmlInfo();
		m_software.reserve(sxi.size());

		for (auto&[equipmentId, si] : sxi)
		{
			Q_ASSERT(equipmentId == si.equipmentID);

			std::shared_ptr<Application> app;

			switch (si.softwareType())
			{
			case E::SoftwareType::Monitor:
				app = std::make_shared<AppMonitor>(si);
				break;
			default:
				app = std::make_shared<Application>(si);
			}

			Q_ASSERT(app);

			app->load(buildPath + equipmentId);

			if (si.softwareType() == E::SoftwareType::TuningService)
			{
				std::shared_ptr<Sim::TuningServiceCommunicator> tsc = std::make_shared<Sim::TuningServiceCommunicator>(m_simulator, equipmentId);

				m_tuningServiceCommunicators.insert({equipmentId, tsc});

				for(const QString& swControllerID : si.softwareControllersIDs)
				{
					m_tuningServiceControllers.insert({swControllerID, tsc});
				}
			}

			m_software.emplace_back(app);
		}

		return ok;
	}

	bool Software::startSimulation(QString profileName)
	{
		bool ok = true;

		ok &= m_appDataTransmitter.startSimulation(profileName);

		for (auto&[id, tcs] : m_tuningServiceCommunicators)
		{
			Q_UNUSED(id);

			ok &= tcs->startSimulation(profileName);
		}

		return ok;
	}

	bool Software::stopSimulation()
	{
		bool ok = true;

		ok &= m_appDataTransmitter.stopSimulation();

		for (auto&[id, tcs] : m_tuningServiceCommunicators)
		{
			Q_UNUSED(id);

			ok &= tcs->stopSimulation();
		}

		return ok;
	}

	QStringList Software::monitors() const
	{
		QStringList result;
		result.reserve(static_cast<int>(m_software.size()));

		for (const auto& app : m_software)
		{
			if (app->softwareType() == E::SoftwareType::Monitor)
			{
				result.push_back(app->equipmentId());
			}
		}

		return result;
	}

	std::shared_ptr<AppMonitor> Software::monitor(QString equipmentId) const
	{
		std::shared_ptr<AppMonitor> result;

		for (const auto& app : m_software)
		{
			if (app->equipmentId() == equipmentId)
			{
				if (app->softwareType() != E::SoftwareType::Monitor)
				{
					Q_ASSERT(app->softwareType() == E::SoftwareType::Monitor);
					break;
				}

				result = std::dynamic_pointer_cast<AppMonitor>(app);
				Q_ASSERT(result);
				break;
			}
		}

		return result;
	}

	bool Software::sendAppData(const QString& lmEquipmentId, const QString& portEquipmentId, const QByteArray& data, TimeStamp timeStamp)
	{
		if (enabled() == false)
		{
			return true;
		}

		return m_appDataTransmitter.sendData(lmEquipmentId, portEquipmentId, data, timeStamp);
	}

	std::shared_ptr<Sim::TuningServiceCommunicator> Software::tuningService(QString equipmentId) const
	{
		std::shared_ptr<Sim::TuningServiceCommunicator> result;

		auto it = m_tuningServiceCommunicators.find(equipmentId);
		if (it != m_tuningServiceCommunicators.end())
		{
			result = it->second;
		}

		auto it2 = m_tuningServiceControllers.find(equipmentId);
		if (it2 != m_tuningServiceControllers.end())
		{
			result = it2->second;
		}

		return result;
	}

	bool Software::enabled() const
	{
		return m_enabled.load();
	}

	void Software::setEnabled(bool value)
	{
		m_enabled = value;
	}

	Sim::AppDataTransmitter& Software::appDataTransmitter()
	{
		return m_appDataTransmitter;
	}

	const Sim::AppDataTransmitter& Software::appDataTransmitter() const
	{
		return m_appDataTransmitter;
	}

	Application::Application(const SoftwareXmlInfo& info) :
		m_info(info)
	{
	}

	bool Application::load(QString /*appDir*/)
	{
		return true;
	}

	const QString& Application::equipmentId() const
	{
		return m_info.equipmentID;
	}

	E::SoftwareType Application::softwareType() const
	{
		return m_info.softwareType();
	}

	const SoftwareXmlInfo& Application::info() const
	{
		return m_info;
	}

	AppMonitor::AppMonitor(const SoftwareXmlInfo& info) :
		Application(info)
	{
	}

	bool AppMonitor::load(QString appDir)
	{
		if (appDir.endsWith('/') == false)
		{
			appDir += '/';
		}

		if (QFile file(appDir + File::GLOBAL_SCRIPT);
			file.open(QIODevice::ReadOnly | QIODevice::Text) == true)
		{
			m_globalScript = file.readAll();
		}

		return true;
	}

	QString AppMonitor::globalScript() const
	{
		return m_globalScript;
	}
}
