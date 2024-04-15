#include "SimSoftwareImpl.h"
#include "SimulatorPrivate.h"

#include <Behavior/ClientBehaviorStorage.h>

namespace  Sim
{

	SoftwareImpl::SoftwareImpl(SimulatorPrivate* simulator) :
		m_simulator(simulator),
		m_log(simulator->log(), "Software"),
		m_appDataTransmitter{simulator}
	{
		Q_ASSERT(m_simulator);
	}

	void SoftwareImpl::clear()
	{
		m_software.clear();
		m_tuningServiceCommunicators.clear();

		return;
	}

	bool SoftwareImpl::load(QString buildPath)
	{
		clear();

		bool ok = loadSoftwareXml(buildPath);

		return ok;
	}

	bool SoftwareImpl::loadSoftwareXml(QString buildPath)
	{
		clear();

		QString softwareFileName = buildPath + Directory::COMMON + "/" + File::SOFTWARE_XML;

		// Raad and parse file /Common/Software.xml
		//
		::SoftwareXmlReader reader;

		bool ok = reader.readSoftwareXml(softwareFileName);
		if (ok == false)
		{
			m_log.writeError(QObject::tr("Load software description error, file %1 not found or corrupted").arg(softwareFileName));
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
				app = std::make_shared<AppMonitorImpl>(si);
				break;
			default:
				app = std::make_shared<Application>(si);
			}

			Q_ASSERT(app);

			ok &= app->load(buildPath + equipmentId, m_log.logInterface());
			
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

	bool SoftwareImpl::startSimulation(QString profileName)
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

	bool SoftwareImpl::stopSimulation()
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

	QStringList SoftwareImpl::monitors() const
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

	std::shared_ptr<AppMonitorImpl> SoftwareImpl::monitor(QString equipmentId) const
	{
		std::shared_ptr<AppMonitorImpl> result;

		for (const auto& app : m_software)
		{
			if (app->equipmentId() == equipmentId)
			{
				if (app->softwareType() != E::SoftwareType::Monitor)
				{
					Q_ASSERT(app->softwareType() == E::SoftwareType::Monitor);
					break;
				}

				result = std::dynamic_pointer_cast<AppMonitorImpl>(app);
				Q_ASSERT(result);
				break;
			}
		}

		return result;
	}

	bool SoftwareImpl::sendAppData(const QString& lmEquipmentId, const QString& portEquipmentId, const QByteArray& data, TimeStamp timeStamp)
	{
		if (enabled() == false)
		{
			return true;
		}

		return m_appDataTransmitter.sendData(lmEquipmentId, portEquipmentId, data, timeStamp);
	}

	std::shared_ptr<Sim::TuningServiceCommunicator> SoftwareImpl::tuningService(QString equipmentId) const
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

	bool SoftwareImpl::enabled() const
	{
		return m_enabled.load();
	}

	void SoftwareImpl::setEnabled(bool value)
	{
		m_enabled = value;
	}

	Sim::AppDataTransmitter& SoftwareImpl::appDataTransmitter()
	{
		return m_appDataTransmitter;
	}

	const Sim::AppDataTransmitter& SoftwareImpl::appDataTransmitter() const
	{
		return m_appDataTransmitter;
	}

	Application::Application(const SoftwareXmlInfo& info) :
		m_info(info)
	{
	}

	bool Application::load(QString /*appDir*/, ILogFile* /*log*/)
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

	AppMonitorImpl::AppMonitorImpl(const SoftwareXmlInfo& info) :
		Application(info)
	{
	}

	bool AppMonitorImpl::load(QString appDir, ILogFile* log)
	{
		if (appDir.endsWith('/') == false)
		{
			appDir += '/';
		}

		// Global script
		//
		if (QFile file(appDir + File::GLOBAL_SCRIPT);
			file.open(QIODevice::ReadOnly | QIODevice::Text) == true)
		{
			m_globalScript = file.readAll();
		}

		// Monitor behavior
		//
		if (QFile file(appDir + File::MONITOR_BEHAVIOR);
			file.open(QIODevice::ReadOnly | QIODevice::Text) == true)
		{
			QByteArray data = file.readAll();

			Behavior::ClientBehaviorStorage storage;
			QString errorMessage;

			bool loadOk = storage.load(data, &errorMessage);
			if (loadOk == false)
			{
				if (log != nullptr)
				{
					log->writeError(QObject::tr("Load monitor behavior error, file %1 corrupted, error %2")
										.arg(appDir + File::MONITOR_BEHAVIOR)
										.arg(errorMessage));
				}

				return false;
			}

			auto behaviors = storage.monitorBehaviors();
			if (behaviors.size() != 1)
			{
				if (log != nullptr)
				{
					log->writeError(QObject::tr("Monitor behavior file %1 must contain one behavior, but it has %2.")
										.arg(appDir + File::MONITOR_BEHAVIOR)
										.arg(behaviors.size()));
				}

				return false;
			}

			m_monitorBehavior = *behaviors.front();
		}

		return true;
	}

	QString AppMonitorImpl::globalScript() const
	{
		return m_globalScript;
	}

	const Behavior::MonitorBehavior& AppMonitorImpl::monitorBehavior() const
	{
		return m_monitorBehavior;
	}
}
