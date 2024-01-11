#include "TuningConfigController.h"
#include "MainWindow.h"
#include "../OnlineLib/SoftwareSettings.h"


//
// ConfigController
//

TuningConfigController::TuningConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile) :
	SchemaClientLib::SchemaClientConfigController{softwareInfo, address1, address2, logFile}
{
	qRegisterMetaType<TuningClientConfigSettings>("TuningClientConfigSettings");
	return;
}

bool TuningConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& conf, const TuningClientSettings& settings, const BuildFileInfoArray& files)
{
	// Copy old settings to new settings, EXCEPT schemas information!
	//
	TuningClientConfigSettings readConfig = configuration();
	readConfig.configurationId = s_configurationIdCounter ++;
	readConfig.configInfo = conf;

	bool apperanceUpdated = readConfig.clientSettings.appearanceChanged(settings);
	bool serversUpdated = readConfig.clientSettings.connectionChanged(settings);

	readConfig.clientSettings = settings;

	// Get all schema details (SchemaDetails.pbuf)
	//
	getSchemasDetails();

	// Get file TUNING_FILTERS
	//
	QByteArray filterData;
	getFileBlockedById(CfgFileId::TUNING_FILTERS, &filterData, nullptr);

	// Get file TUNING_SIGNALS
	//
	QByteArray signalData;
	getFileBlockedById(CfgFileId::TUNING_SIGNALS, &signalData, nullptr);

	// Get file MATS_USERS
	//
	{
		QByteArray matsUsersData;
		getFileBlockedById(CfgFileId::MATSUSERS, &matsUsersData, nullptr);
		
		QString errorString;
		bool ok = readConfig.matsUsers.loadFromByteArray(matsUsersData, errorString);
		if (ok == false)
		{
			m_logFile.writeError(tr("MATS users storage loading failed."));
			readConfig.matsUsers.clear();
		}
	}

	// Get file TUNING_GLOBALSCRIPT
	//
	{
		QByteArray data;
		bool fok = getFileBlockedById(CfgFileId::TUNING_GLOBALSCRIPT, &data, nullptr);

		if (fok == true)
		{
			readConfig.scriptGlobal = QString{data};
		}
		else
		{
			readConfig.scriptGlobal.clear();
		}
	}

	// Check if some files were updated
	//
	bool uiFilesUpdated = false;
	for (const auto& file : files)
	{
		if (file.pathFileName.endsWith("Configuration.xml"))
		{
			continue;
		}

		auto it = m_filesMD5Map.find(file.pathFileName);

		if (it == m_filesMD5Map.end())
		{
			uiFilesUpdated = true;
			m_filesMD5Map[file.pathFileName] = file.md5;
		}
		else
		{
			uiFilesUpdated |= it->second != file.md5;
			it->second = file.md5;
		}
	}

	// Update Configuration
	{
		QWriteLocker locker(&m_lock);
		m_configuration = readConfig;
	}

	// Put dump
	//
	m_logFile.writeMessage(tr("New configuration arrived"));
	dump(readConfig);

	if (uiFilesUpdated == true || apperanceUpdated == true || serversUpdated == true)
	{
		emit signalsArrived(signalData);
		emit filtersArrived(filterData);
		emit configurationArrived(readConfig);
	}

	return true;
}

void TuningConfigController::dump(const TuningClientConfigSettings& conf) const
{
	for (const SoftwareEndpoint::TuningService& ts : conf.clientSettings.tuningServices)
	{
		m_logFile.writeMessage(tr("Tuning Service Connection: %1, %2").arg(ts.shortenId).arg(ts.clientRequestAddress.toString()));
	}

	return;
}

TuningClientConfigSettings TuningConfigController::configuration() const
{
	QReadLocker locker(&m_lock);
	return m_configuration;
}

ClientLib::ConfigurationInfo TuningConfigController::configInfo() const
{
	QReadLocker locker(&m_lock);
	return m_configuration.configInfo;
}

QString TuningConfigController::startSchemaId() const
{
	QReadLocker locker(&m_lock);
	return m_configuration.clientSettings.startSchemaID;
}

bool TuningConfigController::showSignals() const
{
	QReadLocker locker(&m_lock);
	return m_configuration.clientSettings.showSignals;
}

bool TuningConfigController::showSchemas() const
{
	QReadLocker locker(&m_lock);
	return m_configuration.clientSettings.showSchemas;
}

TuningClientSettings::LmStatusFlagMode TuningConfigController::lmStatusFlagMode() const
{
	QReadLocker locker(&m_lock);
	return m_configuration.lmStatusFlagMode();
}

bool TuningConfigController::singleLmControlMode() const
{
	QReadLocker locker(&m_lock);
	for (const SoftwareEndpoint::TuningService& tuns : m_configuration.clientSettings.tuningServices)
	{
		if (tuns.singleLmControl == true)
		{
			return true;
		}
	}
	return false;
}
