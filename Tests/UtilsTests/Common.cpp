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

std::shared_ptr<AppDataReceiver> appDataReceiver;

bool isGTestDeathChild(const QStringList& args)
{
	for(const QString& arg : args)
	{
		if (arg.startsWith("--gtest_internal_run_death_test="))
		{
			return true;
		}
	}

	return false;
}

//

bool loadConfiguration()
{
	QString filePath = buildPath + "/SYSTEMID_RACK01_WS00_ADS/Configuration.xml";

	QFile f(filePath);

	if (!f.open(QIODeviceBase::ReadOnly))
	{
		logMsg(QString("Error read file: %1").arg(filePath));
		return false;
	}

	QByteArray fileData = f.readAll();

	XmlReadHelper xmlReader(fileData);

	bool res = buildInfo.readFromXml(xmlReader);

	if (res == false)
	{
		logMsg("Error read BuildInfo!");
		return false;
	}

	logMsg("BuildInfo read Ok");

	res = settingsSet.readFromXml(xmlReader);

	if (res == false)
	{
		logMsg("Error read SettingsSet!");
		return false;
	}

	logMsg("SettingsSet read Ok");

	std::shared_ptr<const AppDataServiceSettings> st = settingsSet.getSettingsProfile<AppDataServiceSettings>(profileName);

	if (st == nullptr)
	{
		DEBUG_LOG_ERR(logger, QString("Error loading AppDataServiceSettings profile '%1'!").arg(profileName));
		return false;
	}

	logMsg(QString("AppDataServiceSettings profile '%1' read Ok").arg(profileName));

	appDataSrvSettings = *st.get();

	return res;
}

bool loadAppDataSources()
{
	QString filePath = buildPath + "/SYSTEMID_RACK01_WS00_ADS/AppDataSources.xml";

	QFile f(filePath);

	if (!f.open(QIODeviceBase::ReadOnly))
	{
		logMsg(QString("Error read file: %1").arg(filePath));
		return false;
	}

	QByteArray fileData = f.readAll();

	bool res = AppDataSrvTools::readAppDataSources(fileData, profileName, appDataSources, logger);

	if (res == false)
	{
		logMsg("Error loading AppDataSources!");
		return false;
	}

	logMsg("AppDataSources read Ok");

	return res;
}

bool loadAppSignals()
{
	QString filePath = buildPath + "/SYSTEMID_RACK01_WS00_ADS/AcquiredAppSignals.asgs";

	QFile f(filePath);

	if (!f.open(QIODeviceBase::ReadOnly))
	{
		logMsg(QString("Error read file: %1").arg(filePath));
		return false;
	}

	QByteArray fileData = f.readAll();

	fileData = qUncompress(fileData);

	bool res = AppDataSrvTools::readAppSignals(fileData, appSignals);

	if (res == false)
	{
		logMsg("Error loading AppSignals!");
		return false;
	}

	logMsg("AppDataSignals read Ok");

	return res;
}

void createAndInitSignalStates()
{
	AppDataSrvTools::createAndInitSignalStates(appSignals, appSignalStates, 4);
}

void createAndStartAppDataReceiver()
{
	appDataReceiver = std::make_shared<AppDataReceiver>(HostAddressPort("192.168.11.254", PORT_APP_DATA_SERVICE_DATA),
														appDataSources,
														appSignalStates,
														4, E::SoftwareRunMode::Normal,
														logger);

	appDataReceiver->setEnableLog(false);
	appDataReceiver->start();
}

void stopAppDataReceiver()
{
	appDataReceiver->quitAndWait();
}

//

std::shared_ptr<DiscretesLogWriter> startDiscretesLogWriter(const QString& project, const QString& equipmentID)
{
	std::shared_ptr<DiscretesLogWriter> dsLogWriter = std::make_shared<DiscretesLogWriter>();

	dsLogWriter->deleteDatabaseFiles(project, equipmentID);
	dsLogWriter->start(project, equipmentID, 1, logger);

	QThread::sleep(3);

	return dsLogWriter;
}

void stopDiscretesLogWriter(std::shared_ptr<DiscretesLogWriter> dsLogWriter)
{
	dsLogWriter->stop();
	dsLogWriter.reset();
}

void logMsg(const QString& msg)
{
	std::cout << C_STR(QString("%1\n").arg(msg));
}
