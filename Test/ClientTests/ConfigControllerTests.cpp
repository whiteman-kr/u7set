// Functional tests for class ClientLib::Config controller
// ConfigurationService must be ranning on localhost and default port
//
#include "../../ClientLib/ConfigController.h"
#include "../../lib/BuildInfo.h"

namespace
{
	std::atomic<bool> connectedFlag = false;

	std::mutex mutexConnected;
	QString connectionResult;
	std::list<std::tuple<QString, bool, QByteArray>> readFiles;


	class MonitorConfigControllerStub : public ClientLib::ConfigController
	{
	public:
		MonitorConfigControllerStub(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* logFile) :
			ClientLib::ConfigController{softwareInfo, address1, address2, logFile}
		{
		}

	protected:
		virtual bool updateConfiguration(const ClientLib::ConfigurationInfo& conf, const MonitorSettings& settings, const BuildFileInfoArray& files) override
		{
			std::list<std::tuple<QString, bool, QByteArray>> rf;
			QString parsingError;
			QByteArray ba;
			bool ok;

			ok = getFileBlocked("/" + softwareInfo().equipmentID() + "/GlobalScript.js", &ba, &parsingError);
			rf.emplace_back("GlobalScript.js", ok, ba);

			ok = getFileBlocked("/" + softwareInfo().equipmentID() + "/OnConfigurationArrived.js", &ba, &parsingError);
			rf.emplace_back(QString{"OnConfigurationArrived.js"}, ok, ba);

			ok = getFileBlockedById(CfgFileId::LOGO, &ba, &parsingError);
			rf.emplace_back(std::make_tuple("CfgFileId::LOGO", ok, ba));

			{
				std::unique_lock lock{mutexConnected};

				receivedConf = conf;
				receivedSettings = settings;
				receivedFiles = files;

				readFiles = std::move(rf);

				connectionResult = "Ok";
			}

			connectedFlag = true;
			return true;
		}

	public:
		ClientLib::ConfigurationInfo receivedConf;
		MonitorSettings receivedSettings;
		BuildFileInfoArray receivedFiles;
	};

}


TEST(ConfigControllerTests, monitorToConfigControllerConnection)
{
	SoftwareInfo softwareInfo;
	softwareInfo.init(E::SoftwareType::Monitor, "SYSTEMID_CLIENTTEST_WS03_MONITOR", 0, 0);
	HostAddressPort host1{"127.0.0.1", 13312};		// valid address, where cfgservice is expected to run.
	HostAddressPort host2{"192.168.99.103", 13313};	// some unreachable address
	ILogFileStub log;

	MonitorConfigControllerStub configController{softwareInfo, host1, host2, &log};
	configController.start();

	QSignalSpy spy{&configController, &ClientLib::ConfigController::error};

	// Wait for connection for some time
	//
	QElapsedTimer timer;
	timer.start();

	while (timer.hasExpired(3000) == false && connectedFlag == false)
	{
		QCoreApplication::instance()->processEvents();
		QThread::usleep(10);
	}

	// Get and check results
	//
	QString result;
	std::list<std::tuple<QString, bool, QByteArray>> readFilesFromServer;

	{
		std::unique_lock lock{mutexConnected};
		result = connectionResult;
		readFilesFromServer = readFiles;
	}

	ASSERT_FALSE(result.isEmpty());		// Should be "Ok"

	EXPECT_TRUE(spy.isEmpty());

	EXPECT_GE(configController.receivedConf.buildNo, 0);
	EXPECT_EQ(configController.receivedConf.softwareEquipmentId, "SYSTEMID_CLIENTTEST_WS03_MONITOR");
	EXPECT_TRUE(configController.receivedConf.project.startsWith("test_simulator_v"));

	// Test MonitorSettings
	//
	EXPECT_EQ(configController.receivedSettings.configService1.address, HostAddressPort("127.0.0.1", 13312));
	EXPECT_EQ(configController.receivedSettings.configService1.equipmentId, "SYSTEMID_CLIENTTEST_WS01_CFGS");

	EXPECT_EQ(configController.receivedSettings.configService2.address, HostAddressPort("127.0.0.1", 13313));
	EXPECT_EQ(configController.receivedSettings.configService2.equipmentId, "SYSTEMID_CLIENTTEST_WS02_CFGS");

	EXPECT_EQ(configController.receivedSettings.startSchemaId, "STARTSCHEMA");
	EXPECT_EQ(configController.receivedSettings.schemaTags, "applogic;monitor;sometag");

	ASSERT_EQ(configController.receivedSettings.appDataServices.size(), 2);
	EXPECT_EQ(configController.receivedSettings.appDataServices[0].equipmentId, "SYSTEMID_CLIENTTEST_WS01_ADS");
	EXPECT_EQ(configController.receivedSettings.appDataServices[1].equipmentId, "SYSTEMID_CLIENTTEST_WS02_ADS");

	ASSERT_EQ(configController.receivedSettings.archiveServices.size(), 2);
	EXPECT_EQ(configController.receivedSettings.archiveServices[0].equipmentId, "SYSTEMID_CLIENTTEST_WS01_ARCHS");
	EXPECT_EQ(configController.receivedSettings.archiveServices[0].appDataServiceId, "SYSTEMID_CLIENTTEST_WS01_ADS");
	EXPECT_EQ(configController.receivedSettings.archiveServices[1].equipmentId, "SYSTEMID_CLIENTTEST_WS02_ARCHS");
	EXPECT_EQ(configController.receivedSettings.archiveServices[1].appDataServiceId, "SYSTEMID_CLIENTTEST_WS02_ADS");

	EXPECT_EQ(configController.receivedSettings.tuningEnabled, true);
	EXPECT_EQ(configController.receivedSettings.tuningLogin, true);
	EXPECT_EQ(configController.receivedSettings.tuningUserAccounts, "TuningUser1;TuningUser2");
	EXPECT_EQ(configController.receivedSettings.tuningSessionTimeout, 123);

	// Test files
	//
	bool ConfigurationXml = false;
	bool GlobalScriptJs = false;
	bool SchemaDetailsPbuf = false;
	bool TuningSignalsDat = false;

	for (const Builder::BuildFileInfo& file : configController.receivedFiles)
	{
		if (file.pathFileName == "/SYSTEMID_CLIENTTEST_WS03_MONITOR/Configuration.xml")
		{
			ConfigurationXml = true;
			continue;
		}

		if (file.pathFileName == "/SYSTEMID_CLIENTTEST_WS03_MONITOR/GlobalScript.js")
		{
			GlobalScriptJs = true;
			continue;
		}

		if (file.pathFileName == "/SYSTEMID_CLIENTTEST_WS03_MONITOR/SchemaDetails.pbuf")
		{
			SchemaDetailsPbuf = true;
			continue;
		}

		if (file.pathFileName == "/SYSTEMID_CLIENTTEST_WS03_MONITOR/TuningSignals.dat")
		{
			TuningSignalsDat = true;
			EXPECT_EQ(file.ID, CfgFileId::TUNING_SIGNALS);
			continue;
		}
	}

	EXPECT_TRUE(ConfigurationXml);
	EXPECT_TRUE(GlobalScriptJs);
	EXPECT_TRUE(SchemaDetailsPbuf);
	EXPECT_TRUE(TuningSignalsDat);

	EXPECT_FALSE(readFilesFromServer.empty());
	for (const auto&[file, success, data] : readFilesFromServer)
	{
		EXPECT_TRUE(success) << file.toStdString();
	}

	return;
}
