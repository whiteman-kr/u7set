#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <grpcpp/grpcpp.h>
#include <chrono>
#include <thread>
#include <atomic>

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

TEST(GrpcFileSrvTest, GetFile_Short)
{
	std::unique_ptr<GrpcFileSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14100}, server);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	Grpc::GetFileRequest req;

	req.set_filename("/build.xml");

	auto reader = stub->GetFile(&ctx, req);

	Grpc::GetFileReply reply;

	// required string fileName = 1;
	// required int32 errorCode = 2 [default = 0];				// values of Tcp::FileTransferResult
	// required int64 fileSize = 3  [default = 0];
	// required int32 currentPart = 4 [default = 0];
	// required int32 totalParts = 5 [default = 0];
	// required bytes fileData = 6;

	reader->Read(&reply);

	EXPECT_EQ(req.filename(), reply.filename());
	EXPECT_EQ(reply.errorcode(), TO_INT(Tcp::FileTransferResult::Ok));
	EXPECT_EQ(reply.filesize(), reply.filedata().size());
	EXPECT_EQ(reply.currentpart(), 1);
	EXPECT_EQ(reply.totalparts(), 1);

	grpc::Status st = reader->Finish();

	EXPECT_TRUE(st.ok());

//	DEBUG_LOG_MSG(logger, QString("Receive %1 IDs. AppSignals size is %2").arg(got.size()).arg(appSignals.count()));

	server.reset();
}
