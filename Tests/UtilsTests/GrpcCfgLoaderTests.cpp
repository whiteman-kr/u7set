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
		{ "TESTS_GRPC_CFG_LOADER_APP_DATA_SRV", E::SoftwareType::AppDataService, "WS1" },
		{ "TESTS_GRPC_CFG_LOADER_MONITOR", E::SoftwareType::Monitor, "WS1" },
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

TEST(GrpcCfgLoaderTests, WrongHostName)
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
/*
TEST(GrpcFileSrvTest, GetFile_LongFile)
{
	std::unique_ptr<GrpcFileSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14101}, server);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	//

	{
		grpc::ClientContext ctx;

		ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

		Grpc::GetFileRequest req;

		req.set_filename("/Reports/Equipment.json");

		auto reader = stub->GetFile(&ctx, req);

		Grpc::GetFileReply reply;

		QByteArray fileData;

		int curPart = 0;

		while(reader->Read(&reply))
		{
			EXPECT_EQ(req.filename(), reply.filename());
			EXPECT_EQ(reply.errorcode(), TO_INT(Tcp::FileTransferResult::Ok));
			EXPECT_EQ(reply.currentpart(), curPart + 1);
			curPart = reply.currentpart();
			EXPECT_TRUE(reply.currentpart() <= reply.totalparts());

			if (fileData.size() == 0)
			{
				fileData.reserve(reply.filesize());
			}

			const std::string& fData = reply.filedata();

			fileData.append(fData.data(), static_cast<int>(fData.size()));

			if (reply.currentpart() == reply.totalparts())
			{
				break;
			}
		}

		EXPECT_EQ(reply.filesize(), fileData.size());

		QString md5 = Md5Hash::hashStr(fileData);

		QString recvMd5 = QString::fromStdString(reply.md5());

		EXPECT_EQ(md5, recvMd5);

		grpc::Status st = reader->Finish();

		EXPECT_TRUE(st.ok());
	}

	server.reset();
}

TEST(GrpcFileSrvTest, GetFile_WrongFile)
{
	std::unique_ptr<GrpcFileSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14102}, server);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	//

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	Grpc::GetFileRequest req;

	req.set_filename("/build123.xml");			// wrong file

	auto reader = stub->GetFile(&ctx, req);

	Grpc::GetFileReply reply;

	reader->Read(&reply);

	EXPECT_EQ(req.filename(), reply.filename());
	EXPECT_EQ(reply.errorcode(), TO_INT(Tcp::FileTransferResult::RemoteFileIsNotExists));
	EXPECT_EQ(reply.filesize(), 0);
	EXPECT_EQ(reply.currentpart(), 0);
	EXPECT_EQ(reply.totalparts(), 0);
	EXPECT_EQ(reply.filedata().size(), 0);

	grpc::Status st = reader->Finish();

	EXPECT_TRUE(st.ok());

	server.reset();
}

TEST(GrpcFileClientTest, GetFile_ShortFile)
{
	SoftwareInfo serverSw(E::SoftwareType::AppDataService, "TESTS_GRPC_FILE_SRV");

	//HostAddressPort serverAddr("127.0.0.1", 14103);
	HostAddressPort serverAddr("127.0.0.1", 13312);

	std::unique_ptr<GrpcFileSrv> server =
		std::make_unique<GrpcFileSrv>(serverSw, true, std::vector<ClientInfo>{}, false,
									  serverAddr, buildPath, logger);

	const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/temp";

	QDir().mkpath(tempDir);

	//SoftwareInfo clientSw(E::SoftwareType::AppDataService, "TESTS_GRPC_FILE_CLNT");
	SoftwareInfo clientSw(E::SoftwareType::AppDataService, "SYSTEMID_RACK01_WS00_ADS");

	std::unique_ptr<GrpcFileClient> client = std::make_unique<GrpcFileClient>(clientSw,	std::vector<HostAddressPort>{serverAddr},
																			  tempDir, "GrpcFileClientTest", logger);

	QString fileName(File::SLASH_BUILD_XML);

	FileReady fr;
	bool res = client->downloadFileBlocked(fileName, &fr);

	EXPECT_TRUE(res);

	EXPECT_EQ(fr.fileName, fileName);
	EXPECT_EQ(fr.errorCode, Tcp::FileTransferResult::Ok);

	QFile f(tempDir + fileName);

	EXPECT_TRUE(f.exists());

	ASSERT_TRUE(f.open(QIODeviceBase::ReadOnly));

	QByteArray fileData = f.readAll();

	EXPECT_EQ(fileData.size(), fr.fileData.size());

	QString md5 = Md5Hash::hashStr(fileData);

	EXPECT_EQ(md5, fr.md5);

	client.reset();
	server.reset();
}

TEST(GrpcFileClientTest, GetFile_LongFile)
{
	SoftwareInfo serverSw(E::SoftwareType::AppDataService, "TESTS_GRPC_FILE_SRV");

	HostAddressPort serverAddr("127.0.0.1", 14104);

	std::unique_ptr<GrpcFileSrv> server =
		std::make_unique<GrpcFileSrv>(serverSw, true, std::vector<ClientInfo>{}, false,
									  serverAddr, buildPath, logger);

	const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/temp";

	QDir().mkpath(tempDir);

	SoftwareInfo clientSw(E::SoftwareType::AppDataService, "TESTS_GRPC_FILE_CLNT");

	std::unique_ptr<GrpcFileClient> client = std::make_unique<GrpcFileClient>(clientSw,	std::vector<HostAddressPort>{serverAddr},
																			  tempDir, "GrpcFileClientTest", logger);

	QString fileName("/Reports/Equipment.json");

	FileReady fr;
	bool res = client->downloadFileBlocked(fileName, &fr);

	EXPECT_TRUE(res);

	EXPECT_EQ(fr.fileName, fileName);
	EXPECT_EQ(fr.errorCode, Tcp::FileTransferResult::Ok);

	QFile f(tempDir + fileName);

	EXPECT_TRUE(f.exists());

	ASSERT_TRUE(f.open(QIODeviceBase::ReadOnly));

	QByteArray fileData = f.readAll();

	EXPECT_EQ(fileData.size(), fr.fileData.size());

	QString md5 = Md5Hash::hashStr(fileData);

	EXPECT_EQ(md5, fr.md5);

	client.reset();
	server.reset();
}

TEST(GrpcFileClientTest, GetFile_WrongFile)
{
	SoftwareInfo serverSw(E::SoftwareType::AppDataService, "TESTS_GRPC_FILE_SRV");

	HostAddressPort serverAddr("127.0.0.1", 14105);

	std::unique_ptr<GrpcFileSrv> server =
		std::make_unique<GrpcFileSrv>(serverSw, true, std::vector<ClientInfo>{}, false,
									  serverAddr, buildPath, logger);

	const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/temp";

	QDir().mkpath(tempDir);

	SoftwareInfo clientSw(E::SoftwareType::AppDataService, "TESTS_GRPC_FILE_CLNT");

	std::unique_ptr<GrpcFileClient> client = std::make_unique<GrpcFileClient>(clientSw,	std::vector<HostAddressPort>{serverAddr},
																			  tempDir, "GrpcFileClientTest", logger);

	QString fileName("/qqq111.txt");

	FileReady fr;
	bool res = client->downloadFileBlocked(fileName, &fr);

	EXPECT_TRUE(res);

	EXPECT_EQ(fr.fileName, fileName);
	EXPECT_EQ(fr.errorCode, Tcp::FileTransferResult::RemoteFileIsNotExists);
	EXPECT_TRUE(fr.fileData.size() == 0);
	EXPECT_TRUE(fr.md5.size() == 0);

	client.reset();
	server.reset();
}

TEST(GrpcFileClientTest, GetFile_WrongLocalFolder)
{
	SoftwareInfo serverSw(E::SoftwareType::AppDataService, "TESTS_GRPC_FILE_SRV");

	HostAddressPort serverAddr("127.0.0.1", 14106);

	std::unique_ptr<GrpcFileSrv> server =
		std::make_unique<GrpcFileSrv>(serverSw, true, std::vector<ClientInfo>{}, false,
									  serverAddr, buildPath, logger);

	const QString tempDir = "P:/temp";

	SoftwareInfo clientSw(E::SoftwareType::AppDataService, "TESTS_GRPC_FILE_CLNT");

	std::unique_ptr<GrpcFileClient> client = std::make_unique<GrpcFileClient>(clientSw,	std::vector<HostAddressPort>{serverAddr},
																			  tempDir, "GrpcFileClientTest", logger);

	QString fileName(File::SLASH_BUILD_XML);

	FileReady fr;
	bool res = client->downloadFileBlocked(fileName, &fr);
	EXPECT_TRUE(res);

	EXPECT_EQ(fr.fileName, fileName);
	EXPECT_EQ(fr.errorCode, Tcp::FileTransferResult::CantCreateLocalFolder);
	EXPECT_TRUE(fr.fileData.size() == 0);
	EXPECT_TRUE(fr.md5.size() == 0);

	client.reset();
	server.reset();
}
*/
