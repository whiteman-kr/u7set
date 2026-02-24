#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <grpcpp/grpcpp.h>
#include <chrono>
#include <thread>
#include <atomic>

#include <QStandardPaths>

#include "../../ConfigurationService/GrpcCfgServer.h"
#include "../../OnlineLib/GrpcCfgLoader.h"

#include "Common.h"

const std::vector<ClientInfo> clients =
	{
		{ "SYSTEMID_RACK01_WS00_ADS", E::SoftwareType::AppDataService, "WS1" },
		{ "SYSTEMID_RACK01_WS00_MONITOR", E::SoftwareType::Monitor, "WS1" },
	};

std::shared_ptr<GrpcCfgServer> StartGrpcCfgServer(const HostAddressPort& listenIP,
	bool checkHostName = false)
{
	SoftwareInfo si(E::SoftwareType::ConfigurationService, "TESTS_GRPC_CFG_SRV");

	SessionParams sp;
	sp.currentSettingsProfile = SettingsProfile::DEFAULT;
	sp.softwareRunMode = E::SoftwareRunMode::Normal;

	std::shared_ptr<GrpcCfgServer> server = std::make_shared<GrpcCfgServer>(si, sp, clients, checkHostName,
											  listenIP, buildPath, logger);
	return server;
}

TEST(GrpcCfgLoaderTests, WrongHostname)
{
	HostAddressPort serverAddr("127.0.0.1", 14101);

	std::shared_ptr<GrpcCfgServer> server = StartGrpcCfgServer(serverAddr, true);

	{
		SoftwareInfo si(clients[0].softwareType, clients[0].equipmentID);
		si.setHostname("WRONG_HOST");

		GrpcCfgLoaderThread loader(si, 1, serverAddr, {}, logger);

		bool wrongHostName = false;
		QObject::connect(&loader, &GrpcCfgLoaderThread::signal_wrongClientHostname,
				&loader,
				[&wrongHostName]()
				{
					wrongHostName = true;
				});

		loader.start();

		QElapsedTimer t;
		t.start();
		while (!wrongHostName && t.elapsed() < 7000)
		{
			QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
			QThread::msleep(10);
		}

		EXPECT_TRUE(wrongHostName);
	}
}

TEST(GrpcCfgLoaderTests, WrongClientID)
{
	HostAddressPort serverAddr("127.0.0.1", 14102);

	std::shared_ptr<GrpcCfgServer> server = StartGrpcCfgServer(serverAddr, true);

	{
		SoftwareInfo si(clients[0].softwareType, "WRONG_CLIENT_ID");
		si.setHostname(clients[0].hostname);

		GrpcCfgLoaderThread loader(si, 1, serverAddr, {}, logger);

		bool wrongClientID = false;
		QObject::connect(&loader, &GrpcCfgLoaderThread::signal_unknownClientID,
						 &loader,
						 [&wrongClientID]()
						 {
							 wrongClientID = true;
						 });

		loader.start();

		QElapsedTimer t;
		t.start();
		while (!wrongClientID && t.elapsed() < 7000)
		{
			QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
			QThread::msleep(10);
		}

		EXPECT_TRUE(wrongClientID);
	}
}

TEST(GrpcCfgLoaderTests, ConfigurationReady)
{
	HostAddressPort serverAddr("127.0.0.1", 14103);

	std::shared_ptr<GrpcCfgServer> server = StartGrpcCfgServer(serverAddr, true);

	{
		SoftwareInfo si(clients[0].softwareType, clients[0].equipmentID);
		si.setHostname(clients[0].hostname);

		GrpcCfgLoaderThread loader(si, 1, serverAddr, {}, logger);

		loader.clearWorkFolder();

		bool cfgReady = false;
		QObject::connect(&loader, &GrpcCfgLoaderThread::signal_configurationReady,
						 &loader,
						 [&cfgReady]()
						 {
							 cfgReady = true;
						 });

		loader.start();

		QElapsedTimer t;
		t.start();
		while (!cfgReady && t.elapsed() < 7000)
		{
			QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
			QThread::msleep(10);
		}

		EXPECT_TRUE(cfgReady);
	}
}

TEST(GrpcCfgLoaderTests, CheckBuildFiles)
{
	HostAddressPort serverAddr("127.0.0.1", 14104);

	std::shared_ptr<GrpcCfgServer> server = StartGrpcCfgServer(serverAddr, true);

	{
		SoftwareInfo si(clients[0].softwareType, clients[0].equipmentID);
		si.setHostname(clients[0].hostname);

		GrpcCfgLoaderThread loader(si, 1, serverAddr, {}, logger);

		loader.clearWorkFolder();

		bool cfgReady = false;
		BuildFileInfoArray buildFileArray;

		QObject::connect(&loader, &GrpcCfgLoaderThread::signal_configurationReady,
						 &loader,
						 [&cfgReady, &buildFileArray](const QByteArray configurationXmlData,
									 const BuildFileInfoArray buildFileInfoArray,
									 SessionParams sessionParams,
									 std::shared_ptr<const SoftwareSettings> currentSettingsProfile)
						 {
							cfgReady = true;
							buildFileArray = buildFileInfoArray;
						 });

		loader.start();

		QElapsedTimer t;
		t.start();
		while (!cfgReady && t.elapsed() < 7000)
		{
			QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
			QThread::msleep(10);
		}

		EXPECT_TRUE(cfgReady);

		QString file1 = "/SYSTEMID_RACK01_WS00_ADS/AcquiredAppSignals.asgs";

		auto it1 = std::find_if(buildFileArray.begin(), buildFileArray.end(),
								[&file1](const OnlineLib::BuildFileInfo& bfi)
								{
								   return bfi.pathFileName == file1;
								});

		EXPECT_TRUE(it1 != buildFileArray.end());

		QString file2 = "/SYSTEMID_RACK01_WS00_ADS/AppDataSources.xml";

		auto it2 = std::find_if(buildFileArray.begin(), buildFileArray.end(),
								[&file2](const OnlineLib::BuildFileInfo& bfi)
								{
									return bfi.pathFileName == file2;
								});

		EXPECT_TRUE(it2 != buildFileArray.end());
	}
}

TEST(GrpcCfgLoaderTests, GetFileBlocked)
{
	HostAddressPort serverAddr("127.0.0.1", 14105);

	std::shared_ptr<GrpcCfgServer> server = StartGrpcCfgServer(serverAddr, true);

	{
		SoftwareInfo si(clients[1].softwareType, clients[1].equipmentID);
		si.setHostname(clients[1].hostname);

		GrpcCfgLoaderThread loader(si, 1, serverAddr, {}, logger);

		loader.clearWorkFolder();

		bool cfgReady = false;
		BuildFileInfoArray buildFileArray;

		QObject::connect(&loader, &GrpcCfgLoaderThread::signal_configurationReady,
						 &loader,
						 [&cfgReady, &buildFileArray](const QByteArray configurationXmlData,
													  const BuildFileInfoArray buildFileInfoArray,
													  SessionParams sessionParams,
													  std::shared_ptr<const SoftwareSettings> currentSettingsProfile)
						 {
							 cfgReady = true;
							 buildFileArray = buildFileInfoArray;
						 });

		loader.start();

		QElapsedTimer t;
		t.start();
		while (!cfgReady && t.elapsed() < 7000)
		{
			QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
			QThread::msleep(10);
		}

		EXPECT_TRUE(cfgReady);

		for(const OnlineLib::BuildFileInfo& bfi : buildFileArray)
		{
			qDebug() << C_STR(QString("Check file: %1").arg(bfi.pathFileName));

			QByteArray fileData;
			QString errorStr;
			bool res = loader.getFileBlocked(bfi.pathFileName, &fileData, &errorStr);

			EXPECT_TRUE(res);
			EXPECT_TRUE(errorStr.isEmpty());

			//

			if (bfi.compressed == false)
			{
				EXPECT_EQ(bfi.size, TO_QINT64(fileData.size()));
				QString md5 = Md5Hash::hashStr(fileData);
				EXPECT_EQ(bfi.md5, md5);
			}
		}
	}
}

TEST(GrpcCfgLoaderTests, GetFileBlockedByID)
{
	HostAddressPort serverAddr("127.0.0.1", 14106);

	std::shared_ptr<GrpcCfgServer> server = StartGrpcCfgServer(serverAddr, true);

	{
		SoftwareInfo si(clients[0].softwareType, clients[0].equipmentID);
		si.setHostname(clients[0].hostname);

		GrpcCfgLoaderThread loader(si, 1, serverAddr, {}, logger);

		loader.clearWorkFolder();

		bool cfgReady = false;
		BuildFileInfoArray buildFileArray;

		QObject::connect(&loader, &GrpcCfgLoaderThread::signal_configurationReady,
						 &loader,
						 [&cfgReady, &buildFileArray](const QByteArray configurationXmlData,
													  const BuildFileInfoArray buildFileInfoArray,
													  SessionParams sessionParams,
													  std::shared_ptr<const SoftwareSettings> currentSettingsProfile)
						 {
							 cfgReady = true;
							 buildFileArray = buildFileInfoArray;
						 });

		loader.start();

		QElapsedTimer t;
		t.start();
		while (!cfgReady && t.elapsed() < 7000)
		{
			QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
			QThread::msleep(10);
		}

		EXPECT_TRUE(cfgReady);

		for(const OnlineLib::BuildFileInfo& bfi : buildFileArray)
		{
			if (bfi.ID.isEmpty() == true)
			{
				continue;
			}

			qDebug() << C_STR(QString("Check file: %1").arg(bfi.pathFileName));

			QByteArray fileData;
			QString errorStr;
			bool res = loader.getFileBlockedByID(bfi.ID, &fileData, &errorStr);

			EXPECT_TRUE(res);
			EXPECT_TRUE(errorStr.isEmpty());

			//

			if (bfi.compressed == false)
			{
				EXPECT_EQ(bfi.size, TO_QINT64(fileData.size()));
				QString md5 = Md5Hash::hashStr(fileData);
				EXPECT_EQ(bfi.md5, md5);
			}
		}
	}
}

TEST(GrpcCfgLoaderTests, GetWrongFileBlocked)
{
	HostAddressPort serverAddr("127.0.0.1", 14107);

	std::shared_ptr<GrpcCfgServer> server = StartGrpcCfgServer(serverAddr, true);

	{
		SoftwareInfo si(clients[0].softwareType, clients[0].equipmentID);
		si.setHostname(clients[0].hostname);

		GrpcCfgLoaderThread loader(si, 1, serverAddr, {}, logger);

		loader.clearWorkFolder();

		bool cfgReady = false;
		BuildFileInfoArray buildFileArray;

		QObject::connect(&loader, &GrpcCfgLoaderThread::signal_configurationReady,
						 &loader,
						 [&cfgReady, &buildFileArray](const QByteArray configurationXmlData,
													  const BuildFileInfoArray buildFileInfoArray,
													  SessionParams sessionParams,
													  std::shared_ptr<const SoftwareSettings> currentSettingsProfile)
						 {
							 cfgReady = true;
							 buildFileArray = buildFileInfoArray;
						 });

		loader.start();

		QElapsedTimer t;
		t.start();
		while (!cfgReady && t.elapsed() < 7000)
		{
			QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
			QThread::msleep(10);
		}

		EXPECT_TRUE(cfgReady);

		QByteArray fileData;
		QString errorStr;

		bool res = loader.getFileBlocked("/wrong_file_name.txt", &fileData, &errorStr);

		EXPECT_FALSE(res);
		EXPECT_FALSE(errorStr.isEmpty());
	}
}

TEST(GrpcCfgLoaderTests, GetWrongFileBlockedByID)
{
	HostAddressPort serverAddr("127.0.0.1", 14108);

	std::shared_ptr<GrpcCfgServer> server = StartGrpcCfgServer(serverAddr, true);

	{
		SoftwareInfo si(clients[0].softwareType, clients[0].equipmentID);
		si.setHostname(clients[0].hostname);

		GrpcCfgLoaderThread loader(si, 1, serverAddr, {}, logger);

		loader.clearWorkFolder();

		bool cfgReady = false;
		BuildFileInfoArray buildFileArray;

		QObject::connect(&loader, &GrpcCfgLoaderThread::signal_configurationReady,
						 &loader,
						 [&cfgReady, &buildFileArray](const QByteArray configurationXmlData,
													  const BuildFileInfoArray buildFileInfoArray,
													  SessionParams sessionParams,
													  std::shared_ptr<const SoftwareSettings> currentSettingsProfile)
						 {
							 cfgReady = true;
							 buildFileArray = buildFileInfoArray;
						 });

		loader.start();

		QElapsedTimer t;
		t.start();
		while (!cfgReady && t.elapsed() < 7000)
		{
			QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
			QThread::msleep(10);
		}

		EXPECT_TRUE(cfgReady);

		QByteArray fileData;
		QString errorStr;

		bool res = loader.getFileBlockedByID("WRONG_FILE_ID", &fileData, &errorStr);

		EXPECT_FALSE(res);
		EXPECT_FALSE(errorStr.isEmpty());
	}
}
