#include <TestSuiteLib/TestSuiteConfigController.h>

#include "../OnlineLib/SoftwareSettings.h"

#include <QRegularExpression>

// ------------------------- TestSuiteConfigController -----------------------------------

namespace TestSuite
{

	TestSuiteConfigController::TestSuiteConfigController(const SoftwareInfo& softwareInfo,
														 HostAddressPort address1,
														 HostAddressPort address2,
														 ILogFile* appLogFile) :
		ConfigController(softwareInfo, address1, address2, appLogFile)
	{
		qRegisterMetaType<ConfigSettings>("ConfigSettings");

		connect(
			this,
			&ConfigController::error,
			this,
			[this](QString error)
			{
				m_logFile.writeError(QString{"Configuration error: %1"}.arg(error));
				emit configrationError();
				clearData();
			},
			Qt::QueuedConnection);


		return;
	}

	bool TestSuiteConfigController::updateConfiguration(const ClientLib::ConfigurationInfo& conf,
														const ::TestSuiteSettings& settings,
														const std::vector<OnlineLib::BuildFileInfo>& files)
	{
		clearData();

		ConfigSettings config{};
		ConfigData configData{};

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
			config.tuningServices.clear();
		}

		config.login = settings.login;
		config.userAccounts = settings.getUsersAccounts();

		if (config.login == false)
		{
			m_logFile.writeWarning(tr("Testing user authorization is disabled. Tests can be executed without supplying a password!"));
		}

		config.plant = settings.plant;
		config.unit = settings.unit;
		config.system = settings.system;

		static const auto re = QRegularExpression("[;\\s]"); // Separators are whitespace and semicolon
		config.scriptTags = settings.scriptTags.split(re, Qt::SkipEmptyParts);

		// Get test files list
		//
		for (const OnlineLib::BuildFileInfo& buildFileInfo : files)
		{
			if (buildFileInfo.pathFileName.endsWith(".js") == false)
			{
				continue;
			}

			config.scriptFiles.push_back(buildFileInfo.pathFileName);
		}

		std::sort(config.scriptFiles.begin(), config.scriptFiles.end());

		// Get script files form CfgService
		//
		configData.scripts.reserve(config.scriptFiles.size());

		for (const QString& fileName : config.scriptFiles)
		{
			QByteArray data;
			QString errorMsg;

			bool loadResult = getFileBlocked(fileName, &data, &errorMsg);

			if (loadResult == false)
			{
				QString completeErrorMessage = tr("updateConfiguration: Get %1 file error:\n%2").arg(fileName).arg(errorMsg);
				m_logFile.writeError(completeErrorMessage);

				emit configrationError();
				return false;
			}

			QString shortenFileName = fileName;
			shortenFileName.remove(m_softwareInfo.equipmentID());
			while (shortenFileName.startsWith("/"))
			{
				shortenFileName.remove(0, 1);
			}
			m_logFile.writeMessage("Loaded script file: " + shortenFileName);
			configData.scripts.emplace_back(shortenFileName, data);
		}

		// Get file CfgFileId::TUNING_SIGNALS
		//
		if (config.tuningEnabled == true)
		{
			if (bool result = getFileBlockedById(CfgFileId::TUNING_SIGNALS, &config.tuningSignalsFile, nullptr); result == false)
			{
				m_logFile.writeError("Failed to load tuning signal list file: TuningSignals.dat");
				emit configrationError();
				return false;
			}
		}

		// Get file MATS_USERS
		//
		if (config.tuningEnabled == true && config.login == true)
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

		/// Get file CfgFileId::REPORT_TEMPLATES
		//
		QByteArray data;
		if (bool result = getFileBlockedById(CfgFileId::TESTSUITE_REPORTTEMPLATES, &data, nullptr); result == false)
		{
			m_logFile.writeError("Failed to get report templates file: ReportTemplates.dat");
			emit configrationError();
			return false;
		}

		QString errorMsg;
		if (configData.reportTemplates.load(data, &errorMsg) == false)
		{
			m_logFile.writeError("Failed to load report templates file: ReportTemplates.dat");
			emit configrationError();
			return false;
		}

		// Trace received params
		//
		qDebug() << "New configuration arrived.";
		m_logFile.writeMessage(tr("New configuration arrived:"));

		dump(config);

		// SET ARRIVED DATA
		// Cannot move config here as it is used later for `emit configurationArrived(config)`
		//
		setData(config, std::move(configData));

		// Emit signal to inform everybody about new configuration
		//
		emit configurationArrived(config);

		return true;
	}

	void TestSuiteConfigController::dump(const ConfigSettings& config) const
	{
		m_logFile.writeMessage(tr("AppDatService(s): %1.").arg(config.appDataServices.size()));
		qDebug() << "AppDatService(s): " << config.appDataServices.size();

		for (const auto& service : config.appDataServices)
		{
			qDebug() << "Service: id, address: " << service.equipmentId << ", " << service.address.addressPortStr();
			m_logFile.writeMessage(tr("Service: id, address: %1, %2.").arg(service.equipmentId).arg(service.address.addressPortStr()));
		}

		// --
		//
		m_logFile.writeMessage(QString("TuningEnabled = %1").arg(config.tuningEnabled));
		if (config.tuningEnabled == true)
		{
			m_logFile.writeMessage(tr("TuningService(s): %1.").arg(config.tuningServices.size()));
			qDebug() << "TuningService(s): " << config.tuningServices.size();

			for (const auto& ts : config.tuningServices)
			{
				m_logFile.writeMessage(
					tr("TuningService: id, address: %1, %2.").arg(ts.equipmentId).arg(ts.clientRequestAddress.addressPortStr()));
				qDebug() << "TuningService: id, address: " << ts.equipmentId << ", " << ts.clientRequestAddress.addressPortStr();

				m_logFile.writeMessage(tr("TuningSources: %1.").arg(ts.drivenSources.join(", ")));
				qDebug() << "TuningSources: " << ts.drivenSources.join(", ");
			}
		}

		for (const QString& s : config.scriptFiles)
		{
			m_logFile.writeMessage(tr("Script File: %1").arg(s));
		}

		return;
	}

	// IScriptProvider interface
	//
	QStringList TestSuiteConfigController::getScriptFileNames() const
	{
		QReadLocker locker{&m_confugurationLock};
		return m_configuration.scriptFiles;
	}

	std::vector<TestScript> TestSuiteConfigController::getScripts() const
	{
		QReadLocker locker{&m_confugurationLock};
		return m_configData.scripts;
	}

	std::optional<::TestSuite::TestScript> TestSuiteConfigController::getGloablScript() const
	{
		QReadLocker locker{&m_confugurationLock};

		auto it = std::find_if(m_configData.scripts.begin(),
							   m_configData.scripts.end(),
							   [](const TestScript& script)
							   {
								   return script.isGlobalScript() == true;
							   });

		return it != m_configData.scripts.end() ? *it : std::optional<TestScript>{};
	}

	std::optional<::TestSuite::TestScript> TestSuiteConfigController::getScriptByFileName(const QString& fileName) const
	{
		QReadLocker locker{&m_confugurationLock};

		auto it = std::find_if(m_configData.scripts.begin(),
							   m_configData.scripts.end(),
							   [&fileName](const TestScript& script)
							   {
								   return script.fileName() == fileName;
							   });

		return it != m_configData.scripts.end() ? *it : std::optional<TestScript>{};
	}

	// End of IScriptProvider interface

	void TestSuiteConfigController::clearData()
	{
		QWriteLocker locker(&m_confugurationLock);
		m_configuration = {};
		m_configData = {};

		return;
	}

	void TestSuiteConfigController::setData(ConfigSettings configuration, ConfigData configData)
	{
		configuration.configurationId = s_configurationIdCounter++;

		QWriteLocker locker(&m_confugurationLock);
		m_configuration = std::move(configuration);
		m_configData = std::move(configData);

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

	bool TestSuiteConfigController::configurationTuningEnabled() const
	{
		QReadLocker locker(&m_confugurationLock);
		return m_configuration.tuningEnabled;
	}

	ConfigData TestSuiteConfigController::configData() const
	{
		QReadLocker locker(&m_confugurationLock);
		return m_configData;
	}
} // namespace TestSuite
