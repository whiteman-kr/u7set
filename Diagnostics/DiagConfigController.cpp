#include "../OnlineLib/SoftwareSettings.h"
#include "DiagConfigController.h"
#include "DiagnosticsAppSettings.h"
#include "Globals.h"


DiagConfigController::DiagConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile) :
	SchemaClientLib::SchemaClientConfigController{softwareInfo, address1, address2, logFile}
{
	qRegisterMetaType<DiagConfigSettings>("DiagConfigSettings");

	return;
}

bool DiagConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& conf, const DiagnosticsSettings& settings, const BuildFileInfoArray& /*files*/)
{
	DiagConfigSettings config{};

	config.configInfo = conf;

	config.startSchemaId = settings.startSchemaId;

	config.diagDataServices = settings.diagDataServices;
	//config.diagDataRealTimeServices = settings.diagDataServices;
	//config.archiveServices = settings.archiveServices;

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

	config.globalScript = getScriptFunc("/" + DiagnosticsAppSettings::instance().equipmentId() + "/GlobalScript.js");
	config.logoImage = getImageFunc(CfgFileId::LOGO);

	// Get all schema details
	//
	{
		bool ok = getSchemasDetails();
		if (ok == false)
		{
			// The error is reported by getSchemasDetails()... configController, get file.
			//
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
		QWriteLocker locker(&m_configurationLock);
		config.configurationId = s_configurationIdCounter++;
		m_configuration = config;		// Cannot move config here as it is used later for `emit configurationArrived(config)`
	}

	// Emit signal to inform everybody about new configuration
	//
	emit configurationArrived(config);
	emit configurationUpdated();

	return true;
}

void DiagConfigController::dump(const DiagConfigSettings& config) const
{
	qDebug() << "StartSchemaID: " << config.startSchemaId;

	// --
	//
	m_logFile.writeMessage(tr("DiagDatService(s): %1.").arg(config.diagDataServices.size()));
	qDebug() << "DiagDatService(s):";

	for (const auto& service : config.diagDataServices)
	{
		qDebug() << "Service: id, address: " << service.equipmentId << ", " << service.address.addressPortStr();
		m_logFile.writeMessage(tr("Service: id, address: %1, %2.").arg(service.equipmentId).arg(service.address.addressPortStr()));
	}

	// --
	//
	//m_logFile.writeMessage(tr("DiagDataRealTimeService(s): %1.").arg(config.diagDataRealTimeServices.size()));
	//qDebug() << "DiagDataRealTimeService(s):";

	//for (const auto& service : config.diagDataRealTimeServices)
	//{
	//	qDebug() << "Service: id, address: " << service.equipmentId << ", " << service.address.addressPortStr();
	//	m_logFile.writeMessage(tr("Service: id, address: %1, %2.").arg(service.equipmentId).arg(service.address.addressPortStr()));
	//}

	// --
	//
	//m_logFile.writeMessage(tr("ArchiveService(s): %1.").arg(config.archiveServices.size()));
	//qDebug() << "ArchiveService(s):";
	//for (const auto& service : config.archiveServices)
	//{
	//	qDebug() << "Service: id, address: " << service.equipmentId << ", " << service.address.addressPortStr();
	//	m_logFile.writeMessage(tr("Service: id, address: %1, %2.").arg(service.equipmentId).arg(service.address.addressPortStr()));
	//}

	// --
	//
	//m_logFile.writeMessage(QString("TuningEnabled = %1").arg(config.tuningEnabled));
	//if (config.tuningEnabled == true)
	//{
	//	for (const auto& ts : config.tuningServices)
	//	{
	//		m_logFile.writeMessage(tr("TuningService (id, address): %1, %2.").arg(ts.shortenId).arg(ts.clientRequestAddress.toString()));
	//		m_logFile.writeMessage(tr("TuningSources: %1.").arg(ts.drivenSources.join(", ")));
	//	}
	//	m_logFile.writeMessage(tr("TuningUserAccounts: %1.").arg(config.tuningUserAccounts.join(", ")));
	//	m_logFile.writeMessage(tr("TuningSessionTimeout: %1.").arg(config.tuningSessionTimeout));
	//}

	return;
}

int DiagConfigController::configurationId() const
{
	QReadLocker locker(&m_configurationLock);
	return m_configuration.configurationId;
}

DiagConfigSettings DiagConfigController::configuration() const
{
	QReadLocker locker(&m_configurationLock);
	return m_configuration;
}

ClientLib::ConfigurationInfo DiagConfigController::configInfo() const
{
	QReadLocker locker(&m_configurationLock);
	return m_configuration.configInfo;
}

QString DiagConfigController::configurationStartSchemaId() const
{
	QReadLocker locker(&m_configurationLock);
	return m_configuration.startSchemaId;
}
