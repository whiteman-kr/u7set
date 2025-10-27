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

	res = settingsSet.readFromXml(xmlReader);

	if (res == false)
	{
		DEBUG_LOG_ERR(logger, "Error read SettingsSet!");
		return false;
	}

	std::shared_ptr<const AppDataServiceSettings> st = settingsSet.getSettingsProfile<AppDataServiceSettings>(profileName);

	if (st == nullptr)
	{
		DEBUG_LOG_ERR(logger, "Error loading AppDataServiceSettings current profile!");
		return false;
	}

	appDataSrvSettings = *st.get();

	return res;
}

bool loadAppDataSources()
{
	QString filePath = buildPath + "/SYSTEMID_RACK01_WS00_ADS/AppDataSources.xml";

	QFile f(filePath);

	if (!f.open(QIODeviceBase::ReadOnly))
	{
		return false;
	}

	QByteArray fileData = f.readAll();

	bool res = AppDataSrvTools::readAppDataSources(fileData, profileName, appDataSources, logger);

	return res;
}

bool loadAppSignals()
{
	QString filePath = buildPath + "/SYSTEMID_RACK01_WS00_ADS/AcquiredAppSignals.asgs";

	QFile f(filePath);

	if (!f.open(QIODeviceBase::ReadOnly))
	{
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

	return res;
}

void createAndInitSignalStates()
{
	AppDataSrvTools::createAndInitSignalStates(appSignals, appSignalStates, 4);
}
