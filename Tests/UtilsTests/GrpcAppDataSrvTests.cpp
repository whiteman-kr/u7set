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

const std::vector<ClientInfo> clients =
	{
		{ "MONITOR1", E::SoftwareType::Monitor, "WS1" },
		{ "MONITOR2", E::SoftwareType::Monitor, "WS2" },
		{ "MONITOR3", E::SoftwareType::Monitor, "WS3" },
	};

std::unique_ptr<Grpc::AppDataSrv::Stub> StartServerAndMakeClient(const HostAddressPort& listenIP,
																std::unique_ptr<GrpcAppDataSrv>& outServer,
																std::shared_ptr<DiscretesLogWriter> dsLogWriter,
																bool allowAllClients = false,
																bool checkHostName = true)
{
	SoftwareInfo si(E::SoftwareType::AppDataService, "TESTS_GRPC_APP_DATA_SRV");

	if (dsLogWriter == nullptr)
	{
		dsLogWriter = std::make_shared<DiscretesLogWriter>();		// make fake writer
	}

	outServer = std::make_unique<GrpcAppDataSrv>(si, allowAllClients, clients, checkHostName,
												 listenIP, appDataSources, appDataReceiver.get(),
												 appSignals, appSignalStates,
												 dsLogWriter, logger);

	const std::string endpoint = listenIP.addressPortStr().toStdString();

	auto channel = grpc::CreateChannel(endpoint, grpc::InsecureChannelCredentials());
	return Grpc::AppDataSrv::NewStub(channel);
}

std::string Handshake(Grpc::AppDataSrv::Stub& stub, const ClientInfo& ci, grpc::Status* status = nullptr)
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

TEST(GrpcAppDataSrvTest, RunSeveralServersOnSamePort)
{
	std::cout << "Start server 1";
	std::unique_ptr<GrpcAppDataSrv> server1;
	auto stub1 = StartServerAndMakeClient({"127.0.0.1", 13990} , server1, nullptr);

	QThread::sleep(3);

	std::cout << "Start server 2";
	std::unique_ptr<GrpcAppDataSrv> server2;
	auto stub2 = StartServerAndMakeClient({"127.0.0.1", 13990} , server2, nullptr);

	QThread::sleep(3);

	EXPECT_TRUE(server1->isBinded());
	EXPECT_FALSE(server2->isBinded());

	std::cout << "Reset server 1";
	server1.reset();

	std::cout << "Wait while server 2 bind";
	QThread::sleep(7);

	EXPECT_TRUE(server2->isBinded());

	std::cout << "Reset server 2";
	server2.reset();
}

TEST(GrpcAppDataSrvTest, HandshakeNormal)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 13995} , server, nullptr);

	grpc::Status status;

	std::string authToken = Handshake(*stub, clients[0], &status);

	EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
	EXPECT_EQ(authToken.empty(), false);

	server.reset();
}

TEST(GrpcAppDataSrvTest, HandshakeWrongClientID)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 13996} , server, nullptr);

	ClientInfo ci = clients[0];

	ci.equipmentID = "WRONG_CLIENT_ID";

	grpc::Status status;

	std::string authToken = Handshake(*stub, ci, &status);

	EXPECT_EQ(status.error_code(), grpc::StatusCode::UNAUTHENTICATED);
	EXPECT_EQ(status.error_message(), Grpc::WRONG_CLIENT_EQUIPMENT_ID);
	EXPECT_EQ(authToken.empty(), true);

	server.reset();
}

TEST(GrpcAppDataSrvTest, HandshakeWrongClientID_Allowed)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 13997} , server, nullptr,
										 true);	// allow all clients

	ClientInfo ci = clients[0];

	ci.equipmentID = "WRONG_CLIENT_ID";

	grpc::Status status;

	std::string authToken = Handshake(*stub, ci, &status);

	EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
	EXPECT_EQ(authToken.empty(), false);

	server.reset();
}

TEST(GrpcAppDataSrvTest, HandshakeWrongHostName)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 13998} , server, nullptr,
										 false,		// allow all clients - false
										 true);		// check host name - true

	ClientInfo ci = clients[1];

	ci.hostname = "WRONG_HOST";

	grpc::Status status;

	std::string authToken = Handshake(*stub, ci, &status);

	EXPECT_EQ(status.error_code(), grpc::StatusCode::UNAUTHENTICATED);
	EXPECT_EQ(status.error_message(), Grpc::WRONG_HOST_NAME);
	EXPECT_EQ(authToken.empty(), true);

	server.reset();
}

TEST(GrpcAppDataSrvTest, HandshakeWrongHostName_Allowed)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 13999} , server, nullptr, false, false);

	ClientInfo ci = clients[1];

	ci.hostname = "WRONG_HOST";

	grpc::Status status;

	std::string authToken = Handshake(*stub, ci);

	EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
	EXPECT_EQ(authToken.empty(), false);

	server.reset();
}

TEST(GrpcAppDataSrvTest, SessionTimeout)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14000} , server, nullptr);

	server->setSessionTimeout(5);

	std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	QThread::sleep(7);

	Grpc::GetAppSignalStateRequest request;
	Grpc::GetAppSignalStateReply reply;

	grpc::Status status = stub->GetAppSignalState(&ctx, request, &reply);

	EXPECT_EQ(status.error_code(), grpc::StatusCode::UNAUTHENTICATED);
	EXPECT_EQ(status.error_message(), Grpc::INVALID_OR_EXPIRED_SESSION);

	authToken = Handshake(*stub, clients[0], &status);

	EXPECT_TRUE(status.ok());
	ASSERT_FALSE(authToken.empty());

	server.reset();
}

TEST(GrpcAppDataSrvTest, StartsAndStopsCleanly)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14000} , server, nullptr);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

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
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14001}, server, nullptr);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

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
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14002}, server, nullptr);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

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
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14003}, server, nullptr);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

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

TEST(GrpcAppDataSrvTest, GetAppSignalState)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14004}, server, nullptr);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	std::vector<Hash> queryHashes;

	int statesCount = appSignalStates.size();

	qint64 now = currentMSecsUTC();

	for(int i = 0; i < statesCount; i++)
	{
		DynamicAppSignalState* state = appSignalStates[i];

		TEST_PTR_CONTINUE(state);

		queryHashes.push_back(state->hash());

		Times time;

		time.local.timeStamp = now + i;
		time.system.timeStamp = now + i + 10000;
		time.plant.timeStamp = now + i - 10000;

		AppSignalStateFlags f;

		f.all = randomUint32();

		state->setCurrent(time, i, f);
	}

	Grpc::GetAppSignalStateRequest req;

	for (auto h : queryHashes)
	{
		req.add_signalhashes(h);
	}

	Grpc::GetAppSignalStateReply reply;

	grpc::Status st = stub->GetAppSignalState(&ctx, req, &reply);

	EXPECT_TRUE(st.ok());
	EXPECT_EQ(req.signalhashes_size(), reply.appsignalstates_size());

	bool equal = true;

	for(const Proto::AppSignalState& pass : reply.appsignalstates())
	{
		SimpleAppSignalState sass;

		sass.load(pass);

		DynamicAppSignalState* state = appSignalStates.getStateByHash(sass.hash);

		if (state == nullptr)
		{
			Q_ASSERT(false);
			equal = false;
			break;
		}

		equal &= state->current() == sass;
	}

	EXPECT_TRUE(equal);

	server.reset();
}

TEST(GrpcAppDataSrvTest, GetAppSignalState_ExceedHashesCount)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14004}, server, nullptr);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	Grpc::GetAppSignalStateRequest req;

	for (int i = 0; i < ADS_GET_APP_SIGNAL_STATE_MAX + 1; i++)
	{
		req.add_signalhashes(i);			// any hashes
	}

	Grpc::GetAppSignalStateReply reply;

	grpc::Status st = stub->GetAppSignalState(&ctx, req, &reply);

	EXPECT_EQ(st.error_code(), grpc::StatusCode::OUT_OF_RANGE);
	EXPECT_EQ(st.error_message(), Grpc::SIGNAL_HASHES_COUNT_EXEEDS_ADS_GET_APP_SIGNAL_STATE_MAX);

	server.reset();
}

TEST(GrpcAppDataSrvTest, GetAppSignalStateChanges)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14005}, server, nullptr);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	std::thread stateChangesProducerThread = std::thread([]()
	{
		AppDataSource* src = appDataSources.getSourceByEquipmentID("SYSTEMID_RACK01_FSCC01_MD00");

		if (src == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		src->resizeSignalStatesQueue(10000);

		QThread::sleep(3);

		SimpleAppSignalState state;

		for(quint64 i = 1; i < 10000; i++)
		{
			state.hash = i;
			state.time.plant.timeStamp = i + 1000;
			state.time.local.timeStamp = i + 2000;
			state.time.system.timeStamp = i + 3000;
			state.value = (i % 100);
			state.flags.all = i + 101;
			src->pushState(state);

			if ((i % 500) == 0)
			{
				QThread::msleep(30);
			}
		}
	});

	Grpc::GetAppSignalStateChangesRequest req;

	auto reader = stub->GetAppSignalStateChanges(&ctx, req);

	Grpc::GetAppSignalStateChangesReply reply;

	quint64 ctr = 1;

	while (reader->Read(&reply))
	{
		DEBUG_LOG_MSG(logger, QString("Recevie state changes: %1").arg(reply.appsignalstates_size()));

		bool exit = false;

		for(const Proto::AppSignalState& st : reply.appsignalstates())
		{
			EXPECT_EQ(st.hash(),  ctr);
			EXPECT_EQ(st.value(), static_cast<double>(ctr % 100));
			EXPECT_EQ(st.flags(), ctr + 101);
			EXPECT_EQ(st.planttime(), ctr + 1000);
			EXPECT_EQ(st.localtime(), ctr + 2000);
			EXPECT_EQ(st.systemtime(), ctr + 3000);

			if (st.hash() != ctr)
			{
				exit = true;
				break;
			}

			ctr++;
		}

		if (ctr >= 10000 || exit)
		{
			ctx.TryCancel();
			break;
		}
	}

	grpc::Status st = reader->Finish();

	if (stateChangesProducerThread.joinable())
	{
		stateChangesProducerThread.join();
	}

	server.reset();
}

TEST(GrpcAppDataSrvTest, GetDiscretesLog)
{
	std::shared_ptr<DiscretesLogWriter> dsLogWriter = startDiscretesLogWriter("GRPC_TESTS", "EQUIPMENT_ID");

	constexpr int STATES_COUNT = 7000;

	// prefill database
	{
		std::vector<SimpleAppSignalState> states;

		states.reserve(STATES_COUNT);

		for(int i = 1; i < STATES_COUNT; i++)
		{
			SimpleAppSignalState st;

			st.hash = i;
			st.time.plant.timeStamp = i;

			states.push_back(st);
		}

		dsLogWriter->pushStates(states);

		QThread::sleep(3);
	}

	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14006}, server, dsLogWriter);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	Grpc::GetDiscretesLogRequest req;

	auto reader = stub->GetDiscretesLog(&ctx, req);

	Grpc::GetDiscretesLogReply reply;

	quint64 ctr = 1;

	while (reader->Read(&reply))
	{
		DEBUG_LOG_MSG(logger, QString("Recevie discretes log records: %1").arg(reply.discreteslogrecord_size()));

		EXPECT_EQ(reply.logisworkable(), true);
		EXPECT_EQ(reply.logfirstrecordid(), 1);

		for(const Network::DiscretesLogRecord& rd : reply.discreteslogrecord())
		{
			EXPECT_EQ(rd.signalhash(), ctr);
			EXPECT_EQ(rd.planttime(), ctr);

			ctr++;
		}

		if (ctr >= STATES_COUNT)
		{
			break;
		}
	}

	constexpr int UP_CTR = 8000;

	std::thread thread = std::thread([ctr, dsLogWriter]()
	{
		QThread::sleep(1);

		std::vector<SimpleAppSignalState> states;

		states.reserve(200);

		for(int i = ctr; i <= UP_CTR; i++)
		{
			SimpleAppSignalState st;

			st.hash = i;
			st.time.plant.timeStamp = i;

			states.push_back(st);

			if ((i % 100) == 0)
			{
				dsLogWriter->pushStates(states);

				QThread::msleep(500);

				states.clear();
			}
		}
	});

	while (reader->Read(&reply))
	{
		DEBUG_LOG_MSG(logger, QString("Recevie discretes log records: %1").arg(reply.discreteslogrecord_size()));

		EXPECT_EQ(reply.logisworkable(), true);
		EXPECT_EQ(reply.logfirstrecordid(), 1);

		for(const Network::DiscretesLogRecord& rd : reply.discreteslogrecord())
		{
			EXPECT_EQ(rd.signalhash(), ctr);
			EXPECT_EQ(rd.planttime(), ctr);

			ctr++;
		}

		if (ctr >= UP_CTR)
		{
			ctx.TryCancel();
			break;
		}
	}

	grpc::Status st = reader->Finish();

	if (thread.joinable())
	{
		thread.join();
	}

	//

	grpc::ClientContext ctx2;

	ctx2.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	const std::string USER("ACK_USER");
	const std::string SOURCE("ACK_SOURCE");

	Grpc::AckDiscretesLogRequest req2;

	req2.set_ackuser(USER);
	req2.set_acksource(SOURCE);
	req2.set_ackuptoplanttime(2000);

	Grpc::AckDiscretesLogReply rep2;

	st = stub->AckDiscretesLog(&ctx2, req2, &rep2);

	EXPECT_TRUE(st.ok());

	EXPECT_EQ(rep2.ackuser(), USER);
	EXPECT_EQ(rep2.acksource(), SOURCE);
	EXPECT_EQ(rep2.ackuptoplanttime(), 2000);

	//

	QThread::sleep(1);

	grpc::ClientContext ctx3;

	ctx3.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	reader = stub->GetDiscretesLog(&ctx3, req);

	reply.Clear();

	while (reader->Read(&reply))
	{
		EXPECT_EQ(reply.logisworkable(), true);
		EXPECT_EQ(reply.logfirstrecordid(), 2001);
		ctx3.TryCancel();
		break;
	}

	st = reader->Finish();

	server.reset();

	stopDiscretesLogWriter(dsLogWriter);
}

TEST(GrpcAppDataSrvTest, GetAppDataSourcesInfo)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14007}, server, nullptr);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	Grpc::GetAppDataSourcesInfoRequest req;
	Grpc::GetAppDataSourcesInfoReply reply;

	grpc::Status st = stub->GetAppDataSourcesInfo(&ctx, req, &reply);

	EXPECT_TRUE(st.ok());
	EXPECT_EQ(appDataSources.size(), reply.appdatasourceinfo_size());

	for(const Network::DataSourceInfo& dsi : reply.appdatasourceinfo())
	{
		const AppDataSource* appDataSrc = appDataSources.getSourceByEquipmentID(QString::fromStdString(dsi.moduleequipmentid()));

		EXPECT_TRUE(appDataSrc != nullptr);

		if (appDataSrc == nullptr)
		{
			continue;
		}

		Network::DataSourceInfo dsi2;

		appDataSrc->saveToProto(&dsi2);

		EXPECT_EQ(dsi.SerializeAsString(), dsi2.SerializeAsString());
	}

	server.reset();
}

TEST(GrpcAppDataSrvTest, GetAppDataSourcesState)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14008}, server, nullptr);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	Grpc::GetAppDataSourcesStateRequest req;
	Grpc::GetAppDataSourcesStateReply reply;

	grpc::Status st = stub->GetAppDataSourcesState(&ctx, req, &reply);

	EXPECT_TRUE(st.ok());
	EXPECT_EQ(appDataSources.size(), reply.appdatasourcestate_size());

	for(const Network::AppDataSourceState& dsi : reply.appdatasourcestate())
	{
		const AppDataSource* appDataSrc = appDataSources.getSourceByEquipmentID(QString::fromStdString(dsi.lmequipmentid()));

		EXPECT_TRUE(appDataSrc != nullptr);

		if (appDataSrc == nullptr)
		{
			continue;
		}

		Network::AppDataSourceState dsi2;

		appDataSrc->getState(&dsi2);

		EXPECT_EQ(dsi.SerializeAsString(), dsi2.SerializeAsString());
	}

	server.reset();
}

TEST(GrpcAppDataSrvTest, GetServerTime)
{
	std::unique_ptr<GrpcAppDataSrv> server;
	auto stub = StartServerAndMakeClient({"127.0.0.1", 14009}, server, nullptr);

	const std::string authToken = Handshake(*stub, clients[0]);

	ASSERT_FALSE(authToken.empty());

	grpc::ClientContext ctx;

	ctx.AddMetadata(Grpc::SESSION_AUTH_TOKEN, authToken);

	Grpc::GetServerTimeRequest req;
	Grpc::GetServerTimeReply reply;

	grpc::Status st = stub->GetServerTime(&ctx, req, &reply);

	EXPECT_TRUE(st.ok());

	qint64 utc = currentMSecsUTC();
	qint64 local = currentMSecsLocal();

	qDebug() << "received utc:  " << reply.servertimeutc()   << " utc:  " << utc << "diff: " << (utc - reply.servertimeutc());
	qDebug() << "received local:" << reply.servertimelocal() << " local:" << local << "diff: " << (local - reply.servertimelocal());;

	EXPECT_TRUE(reply.servertimeutc() <= utc);
	EXPECT_TRUE(reply.servertimeutc() >= utc - 1000);

	EXPECT_TRUE(reply.servertimelocal() <= local);
	EXPECT_TRUE(reply.servertimelocal() >= local - 1000);

	server.reset();
}
