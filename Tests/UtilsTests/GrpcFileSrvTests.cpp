#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <grpcpp/grpcpp.h>
#include <chrono>
#include <thread>
#include <atomic>

#include <QStandardPaths>

#include "../../AppDataService/GrpcAppDataSrv.h"
#include "../../OnlineLib/SocketIO.h"
#include "../../OnlineLib/TcpFileTransfer.h"
#include "../../OnlineLib/GrpcFileSrv.h"

#include "Common.h"

const std::vector<ClientInfo> clients =
	{
		{ "MONITOR1", E::SoftwareType::Monitor, "WS1" },
	};

std::unique_ptr<Grpc::FileSrv::Stub> StartServerAndMakeClient(const HostAddressPort& listenIP,
																std::unique_ptr<GrpcFileSrv>& outServer)
{
	SoftwareInfo si(E::SoftwareType::AppDataService, "TESTS_GRPC_FILE_SRV");

	outServer = std::make_unique<GrpcFileSrv>(si, true, std::vector<ClientInfo>{}, false,
												 listenIP, buildPath, logger);

	const std::string endpoint = listenIP.addressPortStr().toStdString();

	auto channel = grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials());
	return Grpc::FileSrv::NewStub(channel);
}

std::string Handshake(Grpc::FileSrv::Stub& stub, const ClientInfo& ci, grpc::Status* status = nullptr)
{
	SoftwareInfo si(ci.softwareType, ci.equipmentID);

	si.setHostname(ci.hostname);

	grpc::ClientContext handshakeContext;

	Grpc::HandshakeRequest req;
	Grpc::HandshakeReply rep;

	si.serializeTo(req.mutable_clientsoftwareinfo());

	grpc::Status st = stub.Handshake(&handshakeContext, req, &rep);

	if (status != nullptr)
	{
		*status = st;
	}

	if (st.ok())
	{
		const std::string authToken = rep.authtoken();
		DEBUG_LOG_MSG(logger, QString("Normal handshake, authToken: %1").arg(QString::fromStdString(authToken)));
		return authToken;
	}

	DEBUG_LOG_WRN(logger, QString("Error handshake, status: %1, msg: %2").
						  arg(grpcStatusCodeToString(st.error_code())).
						  arg(QString::fromStdString(st.error_message())));

	return {};
}

TEST(GrpcFileSrvTest, GetFile_ShortFile)
{
	std::unique_ptr<GrpcFileSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14100}, server);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	//

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	Grpc::GetFileRequest req;

	req.set_filename("/build.xml");

	auto reader = stub->GetFile(&ctx, req);

	Grpc::GetFileReply reply;

	reader->Read(&reply);

	EXPECT_EQ(req.filename(), reply.filename());
	EXPECT_EQ(reply.errorcode(), TO_INT(Tcp::FileTransferResult::Ok));
	EXPECT_EQ(reply.filesize(), reply.filedata().size());
	EXPECT_EQ(reply.currentpart(), 1);
	EXPECT_EQ(reply.totalparts(), 1);

	QCryptographicHash md5Gen(QCryptographicHash::Md5);

	md5Gen.addData(QByteArrayView(reply.filedata().data(), reply.filedata().size()));

	QByteArray md5 = md5Gen.result();

	QByteArray recvMd5;

	recvMd5.append(reply.md5().data(), reply.md5().size());

	EXPECT_EQ(md5, recvMd5);

	grpc::Status st = reader->Finish();

	EXPECT_TRUE(st.ok());

	server.reset();
}

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

		QCryptographicHash md5Gen(QCryptographicHash::Md5);

		md5Gen.addData(QByteArrayView(fileData.constData(), fileData.size()));

		QByteArray md5 = md5Gen.result();

		QByteArray recvMd5;

		recvMd5.append(reply.md5().data(), reply.md5().size());

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

	HostAddressPort serverAddr("127.0.0.1", 14103);

	std::unique_ptr<GrpcFileSrv> server =
		std::make_unique<GrpcFileSrv>(serverSw, true, std::vector<ClientInfo>{}, false,
									serverAddr, buildPath, logger);

	const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/temp";

	QDir().mkpath(tempDir);

	SoftwareInfo clientSw(E::SoftwareType::AppDataService, "TESTS_GRPC_FILE_CLNT");

	std::unique_ptr<GrpcFileClient> client = std::make_unique<GrpcFileClient>(clientSw,	std::vector<HostAddressPort>{serverAddr},
						  tempDir, "GrpcFileClientTest", logger);

	QString buildXml("/build.xml");

	FileReady fr;
	bool res = client->downloadFileBlocked(buildXml, &fr);

	EXPECT_TRUE(res);

	EXPECT_EQ(fr.fileName, buildXml);
	EXPECT_EQ(fr.errorCode, Tcp::FileTransferResult::Ok);

	QFile f(tempDir + buildXml);

	EXPECT_TRUE(f.exists());

	QFileInfo fi(f);

	EXPECT_EQ(fi.size(), fr.fileData.size());

	client.reset();

	server.reset();
}


