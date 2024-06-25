#include "SimIdeSimulator.h"
#include "../../lib/ConstStrings.h"
#include <Simulator/Simulator.h>
#include <HardwareLib/DeviceRoot.h>

SimIdeSimulator::SimIdeSimulator(ILogFile* log, bool allowDebugMessages, QObject* parent) :
	m_simulator(new Sim::Simulator(log, allowDebugMessages, parent))
{
}

SimIdeSimulator::~SimIdeSimulator() = default;

bool SimIdeSimulator::load(QString buildPath)
{
	// Save current state of ArminKey, TuningKey. It is convenient feature, so user must not switch on/off tuning options on number of LMs again.
	//
	auto oldLogicModules = m_simulator->logicModules();

	// --
	//
	m_schemaDetails.clear();

	bool ok = true;

	ok &= loadSchemaDetails(buildPath);
	ok &= m_simulator->load(buildPath);

	// Restore state of ArminKey, TuningKey.
	//
	for (auto lms = m_simulator->logicModules();
		 auto& lm : lms)
	{
		auto it = std::find_if(oldLogicModules.begin(), oldLogicModules.end(), [&lm](const auto& oldLm)
							   {
								   return lm.equipmentId() == oldLm.equipmentId();
							   });

		if (it != oldLogicModules.end())
		{
			auto& oldLm = *it;

			lm.setArmingKey(oldLm.armingKey());
			lm.setTuningKey(oldLm.tuningKey());

			lm.setSorSetSwitch1(oldLm.sorSetSwitch1());
			lm.setSorSetSwitch2(oldLm.sorSetSwitch2());
			lm.setSorSetSwitch3(oldLm.sorSetSwitch3());
		}
	}

	// The monitor equipment is a filtered tree of all devices(excluding signals), 
	// which is read from the file Common/MonitorEquipment.dat.
	//
	{
		QFile file{buildPath + "/" + Directory::COMMON + "/" + File::MONITOR_EQUIPMENT};
		if (file.open(QIODevice::ReadOnly) == false)
		{
			log()->writeError("File MONITOR_EQUIPMENT not found or cannot be read.");
			ok = false;
		}

		QByteArray data;
		data = file.readAll();

		std::shared_ptr<Hardware::DeviceObject> rootDeviceObject;

		if (data.isEmpty() == true)
		{
			log()->writeWarning("File MONITOR_EQUIPMENT is empty.");
			
			// Create fake object.
			//
			rootDeviceObject = std::make_shared<Hardware::DeviceRoot>();
		}
		else
		{
			rootDeviceObject = Hardware::DeviceObject::Create(data);
		}

		if (rootDeviceObject == nullptr)
		{
			log()->writeError("Parsing file MONITOR_EQUIPMENT error.");
			ok = false;
		}
		else
		{
			log()->writeMessage(QString("Loaded MONITOR_EQUIPMENT, file %1, size %2").arg(file.fileName()).arg(data.size()));
			m_monitorEquipment = rootDeviceObject;
		}
	}

	// --
	//
	emit projectUpdated();
	return ok;
}

void SimIdeSimulator::clear()
{
	m_schemaDetails.clear();
	m_simulator->clear();

	emit projectUpdated();
	return;
}

const VFrame30::SchemaDetailsSet& SimIdeSimulator::schemaDetails() const
{
	return m_schemaDetails;
}

std::vector<VFrame30::SchemaDetails> SimIdeSimulator::schemasForLm(QString equipmentId) const
{
	return m_schemaDetails.schemasDetails(equipmentId);
}

std::shared_ptr<Hardware::DeviceObject> SimIdeSimulator::monitorEquipment() const
{
	return m_monitorEquipment;
}

bool SimIdeSimulator::isRunning() const
{
	return m_simulator->isRunning();
}

bool SimIdeSimulator::isPaused() const
{
	return m_simulator->isPaused();
}

bool SimIdeSimulator::isStopped() const
{
	return m_simulator->isStopped();
}

ILogFile* SimIdeSimulator::log()
{
	return m_simulator->log();
}

bool SimIdeSimulator::isLoaded() const
{
	return m_simulator->isLoaded();
}

QString SimIdeSimulator::buildPath() const
{
	return m_simulator->buildPath();
}

int SimIdeSimulator::buildNo() const
{
	return m_simulator->buildNo();
}

QString SimIdeSimulator::projectName() const
{
	return m_simulator->projectName();
}

const Sim::Connections& SimIdeSimulator::connections() const
{
	return m_simulator->connections();
}

Sim::Connections& SimIdeSimulator::connections()
{
	return m_simulator->connections();
}

std::vector<Sim::Subsystem> SimIdeSimulator::subsystems() const
{
	return m_simulator->subsystems();
}

std::optional<Sim::LogicModule> SimIdeSimulator::logicModule(QString equipmentId) const
{
	return m_simulator->logicModule(equipmentId);
}

std::vector<Sim::LogicModule> SimIdeSimulator::logicModules() const
{
	return m_simulator->logicModules();
}

Sim::AppSignalManager& SimIdeSimulator::appSignalManager()
{
	return m_simulator->appSignalManager();
}

const Sim::AppSignalManager& SimIdeSimulator::appSignalManager() const
{
	return m_simulator->appSignalManager();
}

ITuningSignalManager& SimIdeSimulator::tuningSignalManagerInterface()
{
	return m_simulator->tuningSignalManagerInterface();
}

const ITuningSignalManager& SimIdeSimulator::tuningSignalManagerInterface() const
{
	return m_simulator->tuningSignalManagerInterface();
}

Sim::OverrideSignals& SimIdeSimulator::overrideSignals()
{
	return m_simulator->overrideSignals();
}

const Sim::OverrideSignals& SimIdeSimulator::overrideSignals() const
{
	return m_simulator->overrideSignals();
}

Sim::Software& SimIdeSimulator::software()
{
	return m_simulator->software();
}

const Sim::Software& SimIdeSimulator::software() const
{
	return m_simulator->software();
}

Sim::Profiles& SimIdeSimulator::profiles()
{
	return m_simulator->profiles();
}

const Sim::Profiles& SimIdeSimulator::profiles() const
{
	return m_simulator->profiles();
}

bool SimIdeSimulator::setCurrentProfile(QString profileName)
{
	return m_simulator->setCurrentProfile(profileName);
}

QString SimIdeSimulator::currentProfileName() const
{
	return m_simulator->currentProfileName();
}

const Sim::Profile& SimIdeSimulator::currentProfile() const
{
	return m_simulator->currentProfile();
}

Sim::Control& SimIdeSimulator::control()
{
	return m_simulator->control();
}

const Sim::Control& SimIdeSimulator::control() const
{
	return m_simulator->control();
}

bool SimIdeSimulator::loadSchemaDetails(QString buildPath)
{
	QString fileName = QDir::fromNativeSeparators(buildPath);
	if (fileName.endsWith(QChar('/')) == false)
	{
		fileName.append(QChar('/'));
	}

	fileName += "Schemas.als/SchemaDetails.pbuf";

	m_simulator->log()->writeMessage(tr("Load logic schema details file: %1").arg(fileName));

	bool ok = true;

	if (QFile::exists(fileName) == false)
	{
		// File not exists, can happen if project does not contain any schemas.
		//
		m_simulator->log()->writeWarning(tr("Project build does not contain any schemas, file %1 not exist.").arg(fileName));

		ok = true;
	}
	else
	{
		ok = m_schemaDetails.Load(fileName);

		if (ok == false)
		{
			m_simulator->log()->writeError(tr("File loading error, file name %1.").arg(fileName));
		}
	}

	emit schemaDetailsUpdated();

	return ok;
}

const Sim::Simulator* SimIdeSimulator::simulator() const
{
	return m_simulator.get();
}

Sim::Simulator* SimIdeSimulator::simulator()
{
	return m_simulator.get();
}

std::vector<VFrame30::SchemaDetails> SimIdeSimulator::schemasDetails() const
{
	std::vector<VFrame30::SchemaDetails> result = m_schemaDetails.schemasDetails();

	return result;
}

std::set<QString> SimIdeSimulator::schemaAppSignals(const QString& schemaId)
{
	std::shared_ptr<VFrame30::SchemaDetails> details = m_schemaDetails.schemaDetails(schemaId);

	if (details == nullptr)
	{
		return std::set<QString>();
	}

	return details->m_signals;
}

QStringList SimIdeSimulator::schemasByAppSignalId(const QString& appSignalId) const
{
	return m_schemaDetails.schemasByAppSignalId(appSignalId);
}

QStringList SimIdeSimulator::schemasByLoopbackId(const QString& loopbackId) const
{
	return m_schemaDetails.schemasByLoopbackId(loopbackId);
}
