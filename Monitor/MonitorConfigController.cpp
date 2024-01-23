#include "../OnlineLib/SoftwareSettings.h"
#include "MonitorConfigController.h"
#include "MonitorAppSettings.h"
#include "Globals.h"


MonitorConfigController::MonitorConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile) :
	ClientLib::ConfigController{softwareInfo, address1, address2, logFile}
{
	qRegisterMetaType<ConfigSettings>("ConfigSettings");

	return;
}

bool MonitorConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& conf, const MonitorSettings& settings, const BuildFileInfoArray& /*files*/)
{
	ConfigSettings config{};

	config.configInfo = conf;

	config.startSchemaId = settings.startSchemaId;

	config.appDataServices = settings.appDataServices;
	config.appDataRealTimeServices = settings.appDataServices;
	config.archiveServices = settings.archiveServices;

	//  --
	//
	config.tuningEnabled = settings.tuningEnabled;

	if (config.tuningEnabled == true)
	{
		config.tuningServices = settings.tuningServices;
		config.tuningLogin = settings.tuningLogin;
		config.tuningUserAccounts = settings.getUsersAccounts();
		config.tuningSessionTimeout = settings.tuningSessionTimeout;
	}
	else
	{
		// tuning disabled
		//
		config.tuningServices.clear();
		config.tuningLogin = false;
		config.tuningUserAccounts.clear();
		config.tuningSessionTimeout = 0;
	}

	//--
	//
	auto getScriptFunc = [this](const QString& scriptFileName) -> QString
		{
			QString parsingError;
			QByteArray ba;

			if (bool ok = getFileBlocked(scriptFileName, &ba, &parsingError);
				ok == true)
			{
				return QString{ba};
			}
			else
			{
				return {};
			}
		};

	// Get image file
	//
	auto getImageFunc = [this](const QString& fileId) -> QPixmap
		{
			QPixmap pixmap;
			QByteArray ba;

			if (bool ok = getFileBlockedById(fileId, &ba, nullptr);
				ok == true)
			{
				pixmap.loadFromData(ba);
			}

			return pixmap;
		};

	config.globalScript = getScriptFunc("/" + MonitorAppSettings::instance().equipmentId() + "/GlobalScript.js");
	config.logoImage = getImageFunc(CfgFileId::LOGO);

	// Get tuning signal files
	//
	if (config.tuningEnabled == true)
	{
		QByteArray data;

		bool result = getFileBlockedById(CfgFileId::TUNING_SIGNALS, &data, nullptr);
		if (result == true)
		{
			emit tuningSignalsArrived(data);
		}
	}

	// Get all schema details
	//
	{
		// Get SchemaDetails.pbuf file
		//
		QByteArray ba;
		QString fileName = "/" + m_softwareInfo.equipmentID() + QStringLiteral("/SchemaDetails.pbuf");

		bool ok = getFileBlocked(fileName, &ba, nullptr);

		QWriteLocker locker(&m_schemaDetailsLock);
		m_schemaDetailsSet.clear();

		if (ok == true)
		{
			m_schemaDetailsSet.Load(ba);
		}
	}

	// New setpoints
	//
	{
		QByteArray data;
		QString errorString;

		if (bool result = getFileBlockedById(CfgFileId::COMPARATOR_SET, &data, &errorString);
			result == false)
		{
			m_logFile.writeError(errorString);
		}
		else
		{
			ComparatorSet setpoints;

			if (bool readOk = setpoints.serializeFrom(data);
				readOk == false)
			{
				m_logFile.writeError(tr("Serialize set point list file error.") + QStringLiteral("\n"));
			}
			else
			{
				m_setpoints = std::move(setpoints);
			}
		}
	}

	// Monitor Behavior
	//
	{
		QByteArray data;
		QString errorString;

		if (bool result = getFileBlockedById(CfgFileId::CLIENT_BEHAVIOR, &data, &errorString);
			result == false)
		{
			m_logFile.writeError("Serialize set point list file error.");
		}
		else
		{
			ClientBehaviorStorage behavior;
			behavior.clear();

			bool ok = behavior.load(data, &errorString);

			if (ok == false)
			{
				m_logFile.writeError("Read/parse Behavior file errror: " + errorString + QStringLiteral("."));
			}
			else
			{
				std::vector<std::shared_ptr<MonitorBehavior>> mb = behavior.monitorBehaviors();

				if (mb.empty() == false)
				{
					config.monitorBeahvior = std::move(*mb[0]);
				}
			}
		}
	}

	// Get file MATS_USERS
	//
	if (config.tuningEnabled == true && config.tuningLogin == true)
	{
		QByteArray matsUsersData;
		getFileBlockedById(CfgFileId::MATSUSERS, &matsUsersData, nullptr);
		
		QString errorString;
		bool ok = config.matsUsers.loadFromByteArray(matsUsersData, errorString);
		if (ok == false)
		{
			m_logFile.writeError(tr("MATS users storage loading failed."));
			config.matsUsers.clear();
		}
	}

	// Trace received params
	//
	qDebug() << "New configuration arrived.";
	m_logFile.writeMessage(tr("New configuration arrived:"));

	dump(config);

	// --
	//
	{
		QWriteLocker locker(&m_confugurationLock);
		config.configurationId = s_configurationIdCounter++;
		m_configuration = config;		// Cannot move config here as it is used later for `emit configurationArrived(config)`
	}

	// Emit signal to inform everybody about new configuration
	//
	emit configurationArrived(config);
	emit configurationUpdated();

	return true;
}

void MonitorConfigController::dump(const ConfigSettings& config) const
{
	qDebug() << "StartSchemaID: " << config.startSchemaId;

	// --
	//
	m_logFile.writeMessage(tr("AppDatService(s): %1.").arg(config.appDataServices.size()));
	qDebug() << "AppDatService(s):";

	for (const auto& service : config.appDataServices)
	{
		qDebug() << "Service: id, address: " << service.equipmentId << ", " << service.address.addressPortStr();
		m_logFile.writeMessage(tr("Service: id, address: %1, %2.").arg(service.equipmentId).arg(service.address.addressPortStr()));
	}

	// --
	//
	m_logFile.writeMessage(tr("AppDataRealTimeService(s): %1.").arg(config.appDataRealTimeServices.size()));
	qDebug() << "AppDataRealTimeService(s):";

	for (const auto& service : config.appDataRealTimeServices)
	{
		qDebug() << "Service: id, address: " << service.equipmentId << ", " << service.address.addressPortStr();
		m_logFile.writeMessage(tr("Service: id, address: %1, %2.").arg(service.equipmentId).arg(service.address.addressPortStr()));
	}

	// --
	//
	m_logFile.writeMessage(tr("ArchiveService(s): %1.").arg(config.archiveServices.size()));
	qDebug() << "ArchiveService(s):";
	for (const auto& service : config.archiveServices)
	{
		qDebug() << "Service: id, address: " << service.equipmentId << ", " << service.address.addressPortStr();
		m_logFile.writeMessage(tr("Service: id, address: %1, %2.").arg(service.equipmentId).arg(service.address.addressPortStr()));
	}

	// --
	//
	m_logFile.writeMessage(QString("TuningEnabled = %1").arg(config.tuningEnabled));
	if (config.tuningEnabled == true)
	{
		for (const auto& ts : config.tuningServices)
		{
			m_logFile.writeMessage(tr("TuningService (id, address): %1, %2.").arg(ts.shortenId).arg(ts.clientRequestAddress.toString()));
			m_logFile.writeMessage(tr("TuningSources: %1.").arg(ts.drivenSources.join(", ")));
		}
		m_logFile.writeMessage(tr("TuningUserAccounts: %1.").arg(config.tuningUserAccounts.join(", ")));
		m_logFile.writeMessage(tr("TuningSessionTimeout: %1.").arg(config.tuningSessionTimeout));
	}

	return;
}

VFrame30::SchemaDetailsSet MonitorConfigController::schemasDetailsSet() const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet;
}

std::vector<VFrame30::SchemaDetails> MonitorConfigController::schemasDetails() const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemasDetails();
}

std::set<QString> MonitorConfigController::schemaAppSignals(const QString& schemaId)
{
	QReadLocker l(&m_schemaDetailsLock);

	std::shared_ptr<VFrame30::SchemaDetails> details = m_schemaDetailsSet.schemaDetails(schemaId);
	if (details == nullptr)
	{
		return {};
	}

	return details->m_signals;
}

QStringList MonitorConfigController::schemasByAppSignalId(const QString& appSignalId) const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemasByAppSignalId(appSignalId);
}

QStringList MonitorConfigController::schemasByLoopbackId(const QString& loopbackId) const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemasByLoopbackId(loopbackId);
}

int MonitorConfigController::configurationId() const
{
	QReadLocker locker(&m_confugurationLock);
	return m_configuration.configurationId;
}

ConfigSettings MonitorConfigController::configuration() const
{
	QReadLocker locker(&m_confugurationLock);
	return m_configuration;
}

ClientLib::ConfigurationInfo MonitorConfigController::configInfo() const
{
	QReadLocker locker(&m_confugurationLock);
	return m_configuration.configInfo;
}

QString MonitorConfigController::configurationStartSchemaId() const
{
	QReadLocker locker(&m_confugurationLock);
	return m_configuration.startSchemaId;
}

int MonitorConfigController::schemaCount() const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemaCount();
}

QString MonitorConfigController::schemaCaptionById(const QString& schemaId) const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemaCaptionById(schemaId);
}

QString MonitorConfigController::schemaCaptionByIndex(int schemaIndex) const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemaCaptionByIndex(schemaIndex);
}

QString MonitorConfigController::schemaIdByIndex(int schemaIndex) const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.schemaIdByIndex(schemaIndex);
}

std::vector<VFrame30::SchemaDetails::TrendIndicatorSchemaItems> MonitorConfigController::trendSchemaItems() const
{
	QReadLocker l(&m_schemaDetailsLock);
	return m_schemaDetailsSet.trendIndicators();
}

const ComparatorSet& MonitorConfigController::setpoints() const
{
	return m_setpoints;
}
