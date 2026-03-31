#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <grpcpp/grpcpp.h>
#include <chrono>
#include <thread>
#include <atomic>

#include "../../AppDataService/GrpcAppDataSrv.h"
#include "../../Metrology/GrpcAdsClient.h"


#include "Common.h"

const std::vector<ClientInfo> clients =
	{
		{ "MONITOR1", E::SoftwareType::Monitor, "WS1" },
		{ "MONITOR2", E::SoftwareType::Monitor, "WS2" },
		{ "MONITOR3", E::SoftwareType::Monitor, "WS3" },
		};

const SoftwareInfo localSwInfo(E::SoftwareType::GatewayService, "TEST_GW_SRV");

std::unique_ptr<GrpcAppDataSrv> StartGrpcAppDataSrv(const HostAddressPort& listenIP)
{
	SoftwareInfo si(E::SoftwareType::AppDataService, "TESTS_GRPC_APP_DATA_SRV");

	return std::make_unique<GrpcAppDataSrv>(si, true, clients, false,
												 listenIP, appDataSources, appDataReceiver.get(),
												 appSignals, appSignalStates, nullptr, logger);
}

std::unique_ptr<GrpcAdsClient> StartGrpcAdsClient(const HostAddressPort& srvAddr,
												const GrpcAdsClient::RequestType stateRequest,
												size_t stateRequestInterval,
												const GrpcAdsClient::RequestType stateChangesRequest,
												size_t stateChangesMaxCount,
												IAppSignalStateUpdaterShared updaterShared)
{
	return std::make_unique<GrpcAdsClient>(localSwInfo,
											std::vector<HostAddressPort>{srvAddr},
											QString("TestGatewayClient"),
											logger, stateRequest, stateRequestInterval,
											stateChangesRequest, stateChangesMaxCount,
											updaterShared);
}

class Updater : public IAppSignalStateUpdater
{
public:
	Updater()
	{

	}

	virtual void adsConnected() override
	{
		connected = true;
	}

	virtual void adsDisconnected() override
	{
		connected = false;
	}

	virtual void updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply) override
	{

	}

	virtual void processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply) override
	{

	}

	virtual void processGatewayAppSignalStateChanges(const Grpc::GetGatewayAppSignalStateChangesReply& reply) override
	{

	}

	void clear()
	{
		connected = false;
	}

public:
	bool connected = false;
};

std::shared_ptr<Updater> updater = std::make_shared<Updater>();

TEST(GrpcAdsClientTests, TestConnection)
{
/*	{
		HostAddressPort srvAddr("127.0.0.1", 14010);

		auto server = StartGrpcAppDataSrv(srvAddr);

		updater->clear();

		auto client = StartGrpcAdsClient(srvAddr,
										 GrpcAdsClient::RequestType::NoRequest, 100,
										 GrpcAdsClient::RequestType::NoRequest, 0,
										 updater);
		client->setPingPeriod(500);
		client->start();

		QThread::msleep(2000);

		EXPECT_TRUE(updater->connected);

		server.reset();

		QThread::msleep(3000);

		EXPECT_FALSE(updater->connected);
	}

	{
		HostAddressPort srvAddr("127.0.0.1", 14010);

		auto server = StartGrpcAppDataSrv(srvAddr);

		updater->clear();

		auto client = StartGrpcAdsClient(srvAddr,
										 GrpcAdsClient::RequestType::GetAppSignalState, 300,
										 GrpcAdsClient::RequestType::NoRequest, 0,
										 updater);
		client->setHashesToRequestStates(std::vector<Hash>{0x0001});
		client->start();

		QThread::msleep(2000);

		EXPECT_TRUE(updater->connected);

		server.reset();

		QThread::msleep(3000);

		EXPECT_FALSE(updater->connected);
	} */

	{
		HostAddressPort srvAddr("127.0.0.1", 14010);

		auto server = StartGrpcAppDataSrv(srvAddr);

		updater->clear();

		auto client = StartGrpcAdsClient(srvAddr,
										 GrpcAdsClient::RequestType::NoRequest, 1000,
										 GrpcAdsClient::RequestType::GetAppSignalStateChanges, 1,
										 updater);
//		client->setHashesToRequestStates(std::vector<Hash>{0x0001});
		client->start();

		QThread::msleep(2000);

		EXPECT_TRUE(updater->connected);

		server.reset();

		QThread::msleep(3000);

		EXPECT_FALSE(updater->connected);
	}

}
