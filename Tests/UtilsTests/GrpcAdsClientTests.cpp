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

std::unique_ptr<GrpcAppDataSrv> createGrpcAppDataSrv(const HostAddressPort& listenIP)
{
	SoftwareInfo si(E::SoftwareType::AppDataService, "TESTS_GRPC_APP_DATA_SRV");

	return std::make_unique<GrpcAppDataSrv>(si, true, clients, false,
												 listenIP, appDataSources, appDataReceiver.get(),
												 appSignals, appSignalStates, nullptr, logger);
}

std::unique_ptr<GrpcAdsClient> createGrpcAdsClient(const HostAddressPort& srvAddr,
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
		SimpleAppSignalState st;

		for(const Proto::AppSignalState& pst : reply.appsignalstates())
		{
			st.load(pst);

			if (st.hash == 0)
			{
				continue;
			}

			states.insert_or_assign(st.hash, st);
		}
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
		states.clear();
	}

	SimpleAppSignalState getState(Hash h) const
	{
		auto it = states.find(h);

		if (it == states.end())
		{
			Q_ASSERT(false);
			return SimpleAppSignalState{};
		}

		SimpleAppSignalState r = it->second;

		return r;
	}

public:
	bool connected = false;
	std::unordered_map<Hash, SimpleAppSignalState> states;
};

std::shared_ptr<Updater> updater = std::make_shared<Updater>();

TEST(GrpcAdsClientTests, TestConnection)
{
	{
		HostAddressPort srvAddr("127.0.0.1", 14010);

		auto server = createGrpcAppDataSrv(srvAddr);
		server->start();

		updater->clear();

		auto client = createGrpcAdsClient(srvAddr,
										 GrpcAdsClient::RequestType::NoRequest, 100,
										 GrpcAdsClient::RequestType::NoRequest, 0,
										 updater);
		client->setPingPeriod(500);
		client->start();

		QThread::msleep(2000);

		EXPECT_TRUE(updater->connected);

		server->stop();

		QThread::msleep(1500);

		EXPECT_FALSE(updater->connected);

		client->stop();
	}

	{
		HostAddressPort srvAddr("127.0.0.1", 14010);

		auto server = createGrpcAppDataSrv(srvAddr);
		server->start();

		updater->clear();

		auto client = createGrpcAdsClient(srvAddr,
										 GrpcAdsClient::RequestType::GetAppSignalState, 300,
										 GrpcAdsClient::RequestType::NoRequest, 0,
										 updater);
		client->setHashesToRequestStates(std::vector<Hash>{0x0001});
		client->start();

		QThread::msleep(2000);

		EXPECT_TRUE(updater->connected);

		server->stop();

		QThread::msleep(1500);

		EXPECT_FALSE(updater->connected);

		client->stop();
	}

	{
		HostAddressPort srvAddr("127.0.0.1", 14010);

		auto server = createGrpcAppDataSrv(srvAddr);
		server->start();

		updater->clear();

		auto client = createGrpcAdsClient(srvAddr,
										 GrpcAdsClient::RequestType::GetAppSignalStateConstSize, 300,
										 GrpcAdsClient::RequestType::NoRequest, 0,
										 updater);
		client->setHashesToRequestStates(std::vector<Hash>{0x0001});
		client->start();

		QThread::msleep(2000);

		EXPECT_TRUE(updater->connected);

		server->stop();

		QThread::msleep(1500);

		EXPECT_FALSE(updater->connected);

		client->stop();
	}

	{
		HostAddressPort srvAddr("127.0.0.1", 14010);

		auto server = createGrpcAppDataSrv(srvAddr);
		server->start();

		updater->clear();

		auto client = createGrpcAdsClient(srvAddr,
										 GrpcAdsClient::RequestType::NoRequest, 1000,
										 GrpcAdsClient::RequestType::GetAppSignalStateChanges, 1,
										 updater);
		client->start();

		QThread::msleep(2000);

		EXPECT_TRUE(updater->connected);

		server->stop();

		QThread::msleep(1500);

		EXPECT_FALSE(updater->connected);

		client->stop();
	}

	{
		HostAddressPort srvAddr("127.0.0.1", 14010);

		auto server = createGrpcAppDataSrv(srvAddr);
		server->start();

		updater->clear();

		auto client = createGrpcAdsClient(srvAddr,
										 GrpcAdsClient::RequestType::NoRequest, 1000,
										 GrpcAdsClient::RequestType::GetGatewayAppSignalStateChanges, 1,
										 updater);
		client->setHashesToRequestGatewayStateChanges(std::vector<Hash>{0x0001});
		client->start();

		QThread::msleep(2000);

		EXPECT_TRUE(updater->connected);

		server->stop();

		QThread::msleep(1500);

		EXPECT_FALSE(updater->connected);

		client->stop();
	}
}

TEST(GrpcAdsClientTests, TestGetAppSignalState)
{
	{
		const QString id1 = "#SYSTEMID_RACK01_FSCC01_MD00_CTRLIN_INL02A";
		const QString id2 = "#LM2_LM1_BLINK";

		const Hash h1 = calcHash(id1);
		const Hash h2 = calcHash(id2);

		HostAddressPort srvAddr("127.0.0.1", 14010);
		auto server = createGrpcAppDataSrv(srvAddr);
		server->start();

		updater->clear();

		auto client = createGrpcAdsClient(srvAddr,
										 GrpcAdsClient::RequestType::GetAppSignalState, 100,
										 GrpcAdsClient::RequestType::NoRequest, 0,
										 updater);

		client->setHashesToRequestStates(std::vector<Hash>{h1, h2});
		client->start();

		QThread::msleep(2000);

		//

		qint64 tm = TO_QINT64(currentMSecsUTC());

		//

		Times times1;

		times1.system.timeStamp = tm;
		times1.local.timeStamp = tm + 2000;
		times1.plant.timeStamp = tm + 4000;

		AppSignalStateFlags flags1;

		flags1.all = 0x1234;

		auto dst1Ptr = appSignalStates.getStateByHash(h1);

		if (dst1Ptr != nullptr)
		{
			dst1Ptr->setCurrent(times1, 34.2, flags1);
		}

		//

		Times times2;

		times2.system.timeStamp = tm + 6000;
		times2.local.timeStamp = tm + 8000;
		times2.plant.timeStamp = tm + 10000;

		AppSignalStateFlags flags2;

		flags2.all = 0x435243;

		auto dst2Ptr = appSignalStates.getStateByHash(h2);

		if (dst2Ptr != nullptr)
		{
			dst2Ptr->setCurrent(times2, 0.1234, flags2);
		}

		//

		QThread::msleep(1000);

		//

		auto st1 = updater->getState(h1);
		auto dst1 = dst1Ptr->current();

		EXPECT_EQ(dst1.time.system.timeStamp, st1.time.system.timeStamp);
		EXPECT_EQ(dst1.time.local.timeStamp, st1.time.local.timeStamp);
		EXPECT_EQ(dst1.time.plant.timeStamp, st1.time.plant.timeStamp);
		EXPECT_EQ(dst1.value, st1.value);
		EXPECT_EQ(dst1.flags.all, st1.flags.all);

		//

		auto st2 = updater->getState(h2);
		auto dst2 = dst2Ptr->current();

		EXPECT_EQ(dst2.time.system.timeStamp, st2.time.system.timeStamp);
		EXPECT_EQ(dst2.time.local.timeStamp, st2.time.local.timeStamp);
		EXPECT_EQ(dst2.time.plant.timeStamp, st2.time.plant.timeStamp);
		EXPECT_EQ(dst2.value, st2.value);
		EXPECT_EQ(dst2.flags.all, st2.flags.all);

		//

		client->stop();
		server->stop();
	}
}

TEST(GrpcAdsClientTests, TestGetAppSignalStateConstSize)
{
	{
		const QString id1 = "#SYSTEMID_RACK01_FSCC01_MD00_CTRLIN_INL02A";
		const QString id2 = "#LM2_LM1_BLINK";

		const Hash h1 = calcHash(id1);
		const Hash h2 = calcHash(id2);

		HostAddressPort srvAddr("127.0.0.1", 14010);
		auto server = createGrpcAppDataSrv(srvAddr);
		server->start();

		updater->clear();

		auto client = createGrpcAdsClient(srvAddr,
										  GrpcAdsClient::RequestType::GetAppSignalStateConstSize, 100,
										  GrpcAdsClient::RequestType::NoRequest, 0,
										  updater);

		constexpr Hash WRONG_HASH = 0x22222;

		client->setHashesToRequestStates(std::vector<Hash>{h1, WRONG_HASH, h2});
		client->start();

		QThread::msleep(2000);

		//

		qint64 tm = TO_QINT64(currentMSecsUTC());

		//

		Times times1;

		times1.system.timeStamp = tm;
		times1.local.timeStamp = tm + 1000;
		times1.plant.timeStamp = tm + 2000;

		AppSignalStateFlags flags1;

		flags1.all = 0x333333;

		auto dst1Ptr = appSignalStates.getStateByHash(h1);

		if (dst1Ptr != nullptr)
		{
			dst1Ptr->setCurrent(times1, 123, flags1);
		}

		//

		Times times2;

		times2.system.timeStamp = tm + 3000;
		times2.local.timeStamp = tm + 4000;
		times2.plant.timeStamp = tm + 5000;

		AppSignalStateFlags flags2;

		flags2.all = 0xFBCDEE;

		auto dst2Ptr = appSignalStates.getStateByHash(h2);

		if (dst2Ptr != nullptr)
		{
			dst2Ptr->setCurrent(times2, 321.4, flags2);
		}

		//

		QThread::msleep(1000);

		//

		auto st1 = updater->getState(h1);
		auto dst1 = dst1Ptr->current();

		EXPECT_EQ(dst1.time.system.timeStamp, st1.time.system.timeStamp);
		EXPECT_EQ(dst1.time.local.timeStamp, st1.time.local.timeStamp);
		EXPECT_EQ(dst1.time.plant.timeStamp, st1.time.plant.timeStamp);
		EXPECT_EQ(dst1.value, st1.value);
		EXPECT_EQ(dst1.flags.all, st1.flags.all);

		//

		auto st2 = updater->getState(h2);
		auto dst2 = dst2Ptr->current();

		EXPECT_EQ(dst2.time.system.timeStamp, st2.time.system.timeStamp);
		EXPECT_EQ(dst2.time.local.timeStamp, st2.time.local.timeStamp);
		EXPECT_EQ(dst2.time.plant.timeStamp, st2.time.plant.timeStamp);
		EXPECT_EQ(dst2.value, st2.value);
		EXPECT_EQ(dst2.flags.all, st2.flags.all);

		//

		client->stop();
		server->stop();
	}
}

