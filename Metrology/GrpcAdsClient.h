#pragma once

#include <GrpcAppDataSrv.grpc.pb.h>

#include "../OnlineLib/GrpcClient.h"

class IAppSignalStateUpdater
{
public:
	virtual ~IAppSignalStateUpdater() = default;

	virtual void updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply) = 0;
	virtual void processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply) = 0;
};

using IAppSignalStateUpdaterShared = std::shared_ptr<IAppSignalStateUpdater>;

class GrpcAdsClient : public GrpcClient<Grpc::AppDataSrv>
{
	Q_OBJECT

public:
	GrpcAdsClient(const SoftwareInfo& localSoftwareInfo,
				  const std::vector<HostAddressPort>& serverAddress,
				  const QString& clientDescription,
				  CircularLoggerShared log,
				  IAppSignalStateUpdaterShared updater);
	virtual ~GrpcAdsClient();

	void setHashesToRequestStates(const std::vector<Hash>& hashes);

private:
	virtual void run() override;

	void getStateRequest(Grpc::GetAppSignalStateRequest* request, bool* isLastPart);

	bool sendGetAppSignalStateRequest(const Grpc::GetAppSignalStateRequest& request);

private:
	IAppSignalStateUpdaterShared m_updater;

	std::mutex m_hashesToRequestStatesMutex;
	std::vector<Hash> m_hashesToRequestStates;
	size_t m_requestStateHashesStartIndex = 0;

	int64_t m_lastRequestTime = 0;
};

