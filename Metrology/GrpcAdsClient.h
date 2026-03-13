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
	enum class RequestType
	{
		GetAppSignalState = 0x0001,
		GetappSignalStateConstSize = 0x0002,
		GetAppSignalStateChanges = 0x0004,
		GatewayGetAppSignalStateChanges = 0x0008
	};

public:
	GrpcAdsClient(const SoftwareInfo& localSoftwareInfo,
				  const std::vector<HostAddressPort>& serverAddress,
				  const QString& clientDescription,
				  CircularLoggerShared log,
				  IAppSignalStateUpdaterShared updater);
	virtual ~GrpcAdsClient();

	void setHashesToRequestStates(const std::vector<Hash>& hashes);

	void setRequestTypes(const std::vector<RequestType>& requestTypes);
	void setStateRequestInterval(qint64 intervalMs);

private:
	virtual void run() override;

	RequestType getNextRequestType(bool& typesRestarted);
	void updateLastRequestTime(int64_t lastRequestTime = currentMSecsUTC());

	void fillGetStateRequest(Grpc::GetAppSignalStateRequest* request, bool* isLastPart);

	bool sendGetAppSignalStateRequest(const Grpc::GetAppSignalStateRequest& request);

private:
	IAppSignalStateUpdaterShared m_updater;

	std::mutex m_requestTypesMutex;
	std::vector<RequestType> m_requestTypes = { RequestType::GetAppSignalState };
	size_t m_requestTypeIndex = 0;

	std::atomic<qint64> m_stateRequestInterval = 200;

	std::mutex m_hashesToRequestStatesMutex;
	std::vector<Hash> m_hashesToRequestStates;
	size_t m_requestStateHashesStartIndex = 0;

	int64_t m_lastRequestTime = 0;
};

