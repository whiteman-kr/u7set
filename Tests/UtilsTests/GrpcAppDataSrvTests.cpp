// GrpcAppDataSrv.tests.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <grpcpp/grpcpp.h>
#include <chrono>
#include <thread>
#include <atomic>

#include "../../AppDataService/GrpcAppDataSrv.h"
#include "../../OnlineLib/SocketIO.h"

#include "Common.h"

std::unique_ptr<Grpc::AppDataSrv::Stub> StartServerAndMakeClient(const HostAddressPort& listenIP,
																std::unique_ptr<GrpcAppDataSrv>& outServer)
{
	outServer = std::make_unique<GrpcAppDataSrv>(listenIP,
												 appSignals,
												 appSignalStates,
												 logger);

	const std::string endpoint = listenIP.addressPortStr().toStdString();

	auto channel = grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials());
	return Grpc::AppDataSrv::NewStub(channel);
}

bool checkReceivedParams(const std::vector<Proto::AppSignal>& recvParams)
{
	bool res = true;

	for(const Proto::AppSignal& p : recvParams)
	{
		AppSignal pas;

		pas.loadFromProto(p);

		const AppSignal* as = appSignals.getByAppSignalID(pas.appSignalID());

		if (as == nullptr)
		{
			DEBUG_LOG_ERR(logger, QString("checkReceivedParams: AppSignal %1 not found").arg(pas.appSignalID()));
			res = false;
			break;
		}

		bool r = as->equalWithAppSignal(pas);

		if (r == false)
		{
			DEBUG_LOG_ERR(logger, QString("checkReceivedParams: AppSignal %1 not equal").arg(pas.appSignalID()));
			res = false;
		}
	}

	return res;
}

TEST(GrpcAppDataSrvTest, StartsAndStopsCleanly)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14000} , server);

	grpc::ClientContext ctx;
	Grpc::GetAppSignalListRequest req;
	std::unique_ptr<grpc::ClientReader<Grpc::GetAppSignalListReply>> reader =
		stub->GetAppSignalList(&ctx, req);

	Grpc::GetAppSignalListReply reply;

	bool any = false;

	while (reader->Read(&reply))
	{
		any = true;
	}

	grpc::Status st = reader->Finish();

	EXPECT_TRUE(st.ok());
	EXPECT_TRUE(any);

	server.reset();
}

TEST(GrpcAppDataSrvTest, GetAppSignalList_ReturnsAllIds)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14001}, server);

	grpc::ClientContext ctx;
	Grpc::GetAppSignalListRequest req;

	auto reader = stub->GetAppSignalList(&ctx, req);

	std::unordered_set<std::string> got;
	Grpc::GetAppSignalListReply reply;

	while (reader->Read(&reply))
	{
		for (const auto& id : reply.appsignalids())
		{
			got.insert(id);
		}
	}

	grpc::Status st = reader->Finish();

	EXPECT_TRUE(st.ok());

	DEBUG_LOG_MSG(logger, QString("Receive %1 IDs. AppSignals size is %2").arg(got.size()).arg(appSignals.count()));

	EXPECT_EQ(static_cast<int>(got.size()), appSignals.count());

	bool res = true;

	for(const auto& id : got)
	{
		if (appSignals.containsAppSignalID(QString::fromStdString(id)) == false)
		{
			res = false;
			break;
		}
	}

	EXPECT_EQ(res, true);

	for(const AppSignal& as : appSignals)
	{
		if (got.contains(as.appSignalID().toStdString()) == false)
		{
			res = false;
			break;
		}
	}

	EXPECT_EQ(res, true);

	server.reset();
}

TEST(GrpcAppDataSrvTest, GetAppSignalParam_AllSignals)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14002}, server);

	grpc::ClientContext ctx;
	Grpc::GetAppSignalParamRequest req;			// hashes size 0 - request ALL signal params
	auto reader = stub->GetAppSignalParam(&ctx, req);

	int total = 0;

	Grpc::GetAppSignalParamReply reply;

	std::vector<Proto::AppSignal> receivedParams;

	receivedParams.reserve(appSignals.count());

	while (reader->Read(&reply))
	{
		EXPECT_EQ(static_cast<int>(reply.totalsize()), appSignals.count());
		EXPECT_EQ(static_cast<int>(reply.replysignalindex()), total);

		total += reply.signalparams_size();

		for(const Proto::AppSignal& pas : reply.signalparams())
		{
			receivedParams.emplace_back(pas);
		}
	}

	grpc::Status st = reader->Finish();

	EXPECT_TRUE(st.ok());

	EXPECT_EQ(total, appSignals.count());

	// check received params

	EXPECT_EQ(checkReceivedParams(receivedParams), true);

	server.reset();
}

TEST(GrpcAppDataSrvTest, GetAppSignalParam_ByHashes)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14003}, server);

	std::vector<Hash> queryHashes;

	constexpr int COUNT = 15;

	queryHashes.reserve(COUNT);

	for(int i = 0; i < COUNT; i++)
	{
		const AppSignal* as = appSignals.getByIndex(randomUint32() % appSignals.count());
		queryHashes.push_back(calcHash(as->appSignalID()));
	}

	Grpc::GetAppSignalParamRequest req;

	for (auto h : queryHashes)
	{
		req.add_signalhashes(h);
	}

	grpc::ClientContext ctx;

	auto reader = stub->GetAppSignalParam(&ctx, req);

	std::vector<Proto::AppSignal> receivedParams;

	receivedParams.reserve(COUNT);

	Grpc::GetAppSignalParamReply reply;

	while (reader->Read(&reply))
	{
		for (const Proto::AppSignal& pas : reply.signalparams())
		{
			receivedParams.emplace_back(pas);
		}
	}

	grpc::Status st = reader->Finish();

	EXPECT_TRUE(st.ok());
	EXPECT_EQ(static_cast<int>(receivedParams.size()), static_cast<int>(queryHashes.size()));

	// check received params

	EXPECT_EQ(checkReceivedParams(receivedParams), true);

	server.reset();
}
