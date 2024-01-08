#include "TuningConfigController.h"
#include "MainWindow.h"
#include "../OnlineLib/SoftwareSettings.h"


//
// ConfigController
//

TuningConfigController::TuningConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile) :
	ClientLib::ConfigController{softwareInfo, address1, address2, logFile}
{
	qRegisterMetaType<ConfigSettings>("ConfigSettings");

	return;
}

int TuningConfigController::schemaCount() const
{
	QReadLocker l(&m_lock);
	return m_schemaDetailsSet.schemaCount();
}

QString TuningConfigController::schemaCaptionById(const QString& schemaId) const
{
	QReadLocker l(&m_lock);
	return m_schemaDetailsSet.schemaCaptionById(schemaId);
}

QString TuningConfigController::schemaCaptionByIndex(int schemaIndex) const
{
	QReadLocker l(&m_lock);
	return m_schemaDetailsSet.schemaCaptionByIndex(schemaIndex);
}

QString TuningConfigController::schemaIdByIndex(int schemaIndex) const
{
	QReadLocker l(&m_lock);
	return m_schemaDetailsSet.schemaIdByIndex(schemaIndex);
}

std::set<QString> TuningConfigController::schemaTagsByIndex(int schemaIndex) const
{
	QReadLocker l(&m_lock);
	auto details = m_schemaDetailsSet.schemaDetails(schemaIndex);
	if (details == nullptr)
	{
		Q_ASSERT(details);
		return {};
	}
	return details->schemaTags();
}


bool TuningConfigController::schemaHasTags(int schemaIndex, const QStringList& tags) const
{
	QReadLocker l(&m_lock);
	auto details = m_schemaDetailsSet.schemaDetails(schemaIndex);
	if (details == nullptr)
	{
		Q_ASSERT(details);
		return false;
	}

	const std::set<QString>& detailsTags = details->schemaTags();
	for (const QString& tag : tags)
	{
		if (detailsTags.find(tag.trimmed().toLower()) != detailsTags.end())
		{
			return true;
		}
	}
	return false;
}

bool TuningConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& conf, const TuningClientSettings& settings, const BuildFileInfoArray& files)
{
	// Copy old settings to new settings, EXCEPT schemas information!
	//
	ConfigSettings readConfig = configuration();
	readConfig.configurationId = s_configurationIdCounter ++;
	readConfig.configInfo = conf;

	bool apperanceUpdated = readConfig.clientSettings.appearanceChanged(settings);
	bool serversUpdated = readConfig.clientSettings.connectionChanged(settings);

	readConfig.clientSettings = settings;

	// Get all schema details
	//
	{
		// Get SchemaDetails.pbuf file
		//
		QByteArray ba;
		QString fileName = "/" + m_softwareInfo.equipmentID() + QStringLiteral("/SchemaDetails.pbuf");

		bool ok = getFileBlocked(fileName, &ba, nullptr);

		QWriteLocker locker(&m_lock);
		m_schemaDetailsSet.clear();

		if (ok == true)
		{
			m_schemaDetailsSet.Load(ba);
		}
	}

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

	// Update Configuratione
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

void TuningConfigController::dump(const ConfigSettings& conf) const
{
	for (const SoftwareEndpoint::TuningService& ts : conf.clientSettings.tuningServices)
	{
		m_logFile.writeMessage(tr("Tuning Service Connection: %1, %2").arg(ts.shortenId).arg(ts.clientRequestAddress.toString()));
	}

	return;
}

ConfigSettings TuningConfigController::configuration() const
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
