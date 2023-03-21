#include "../OnlineLib/SoftwareSettings.h"
#include "TestSuiteConfigController.h"

// ------------------------- TestSuiteConfigController -----------------------------------

TestSuiteConfigController::TestSuiteConfigController(const SoftwareInfo& softwareInfo,
													 HostAddressPort address1,
													 HostAddressPort address2,
													 ILogFile* appLogFile) :
	ConfigController(softwareInfo, address1, address2, appLogFile)
{
	qRegisterMetaType<ConfigSettings>("ConfigSettings");

	return;
}

bool TestSuiteConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& conf, const TestSuiteSettings& settings, const BuildFileInfoArray& files)
{
	ConfigSettings config{};

	config.configInfo = conf;

	config.appDataServices = settings.appDataServices;
	//  --
	//
	config.tuningEnabled = settings.tuningEnabled;

	if (config.tuningEnabled == true)
	{
		config.tuningServices = settings.tuningServices;
	}
	else
	{
		// tuning disabled
		//
		config.tuningServices.clear();
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

	// Get test files list

	for (const Builder::BuildFileInfo& buildFileInfo: files)
	{
		if (buildFileInfo.pathFileName.endsWith(".js") == false)
		{
			continue;
		}

		config.scriptFiles.push_back(buildFileInfo.pathFileName);
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

	return true;
}

void TestSuiteConfigController::dump(const ConfigSettings& config) const
{
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
	m_logFile.writeMessage(QString("TuningEnabled = %.1").arg(config.tuningEnabled));
	if (config.tuningEnabled == true)
	{
		for (const auto& ts : config.tuningServices)
		{
			m_logFile.writeMessage(tr("TuningService (id, ip, port): %1, %2, %3.").arg(ts.equipmentId).arg(ts.clientRequestIP).arg(ts.clientRequestPort));
			m_logFile.writeMessage(tr("TuningSources: %1.").arg(ts.drivenSources.join(", ")));
		}
	}

	for (const QString& s : config.scriptFiles)
	{
		m_logFile.writeMessage(tr("Script File: %1").arg(s));
	}

	return;
}


ConfigSettings TestSuiteConfigController::configuration() const
{
	QReadLocker locker(&m_confugurationLock);
	return m_configuration;
}

ClientLib::ConfigurationInfo TestSuiteConfigController::configInfo() const
{
	QReadLocker locker(&m_confugurationLock);
	return m_configuration.configInfo;
}
