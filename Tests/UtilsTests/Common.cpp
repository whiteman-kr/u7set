#include "Common.h"
#include "../../AppDataService/AppDataSrvTools.h"

CircularLoggerShared logger;

QString buildPath;
QString profileName;

OnlineLib::BuildInfo buildInfo;
SoftwareSettingsSet settingsSet;
AppDataServiceSettings appDataSrvSettings;

AppDataSources appDataSources;
AppSignals appSignals;

DynamicAppSignalStates appSignalStates;

//

bool loadConfiguration()
{
	QString filePath = buildPath + "/SYSTEMID_RACK01_WS00_ADS/Configuration.xml";

	QFile f(filePath);

	if (!f.open(QIODeviceBase::ReadOnly))
	{
		DEBUG_LOG_ERR(logger, QString("Error read file: %1").arg(filePath));
		return false;
	}

	QByteArray fileData = f.readAll();

	XmlReadHelper xmlReader(fileData);

	bool res = buildInfo.readFromXml(xmlReader);

	if (res == false)
	{
		DEBUG_LOG_ERR(logger, "Error read BuildInfo!");
		return false;
	}

	DEBUG_LOG_MSG(logger, "BuildInfo read Ok");

	res = settingsSet.readFromXml(xmlReader);

	if (res == false)
	{
		DEBUG_LOG_ERR(logger, "Error read SettingsSet!");
		return false;
	}

	DEBUG_LOG_MSG(logger, "SettingsSet read Ok");

	std::shared_ptr<const AppDataServiceSettings> st = settingsSet.getSettingsProfile<AppDataServiceSettings>(profileName);

	if (st == nullptr)
	{
		DEBUG_LOG_ERR(logger, QString("Error loading AppDataServiceSettings profile '%1'!").arg(profileName));
		return false;
	}

	DEBUG_LOG_MSG(logger, QString("AppDataServiceSettings profile '%1' read Ok").arg(profileName));

	appDataSrvSettings = *st.get();

	return res;
}

bool loadAppDataSources()
{
	QString filePath = buildPath + "/SYSTEMID_RACK01_WS00_ADS/AppDataSources.xml";

	QFile f(filePath);

	if (!f.open(QIODeviceBase::ReadOnly))
	{
		DEBUG_LOG_ERR(logger, QString("Error read file: %1").arg(filePath));
		return false;
	}

	QByteArray fileData = f.readAll();

	bool res = AppDataSrvTools::readAppDataSources(fileData, profileName, appDataSources, logger);

	if (res == false)
	{
		DEBUG_LOG_ERR(logger, "Error loading AppDataSources!");
		return false;
	}

	DEBUG_LOG_MSG(logger, "AppDataSources read Ok");

	return res;
}

bool loadAppSignals()
{
	QString filePath = buildPath + "/SYSTEMID_RACK01_WS00_ADS/AcquiredAppSignals.asgs";

	QFile f(filePath);

	if (!f.open(QIODeviceBase::ReadOnly))
	{
		DEBUG_LOG_ERR(logger, QString("Error read file: %1").arg(filePath));
		return false;
	}

	QByteArray fileData = f.readAll();

	fileData = qUncompress(fileData);

	bool res = AppDataSrvTools::readAppSignals(fileData, appSignals);

	if (res == false)
	{
		DEBUG_LOG_ERR(logger, "Error loading AppSignals!");
		return false;
	}

	DEBUG_LOG_MSG(logger, "AppDataSignals read Ok");

	return res;
}

void createAndInitSignalStates()
{
	AppDataSrvTools::createAndInitSignalStates(appSignals, appSignalStates, 4);
}
