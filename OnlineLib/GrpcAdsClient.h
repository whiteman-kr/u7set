#pragma once

#include <GrpcAppDataSrv.grpc.pb.h>

#include "../OnlineLib/GrpcClient.h"

class IAppSignalStateUpdater
{
public:
	virtual ~IAppSignalStateUpdater() = default;

	virtual void adsConnected() = 0;
	virtual void adsDisconnected() = 0;

	virtual void updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply) = 0;
	virtual void processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply) = 0;
	virtual void processGatewayAppSignalStateChanges(const Grpc::GetGatewayAppSignalStateChangesReply& reply) = 0;
};

using IAppSignalStateUpdaterShared = std::shared_ptr<IAppSignalStateUpdater>;

class GrpcAdsClient : public GrpcClient<Grpc::AppDataSrv>
{
	Q_OBJECT

public:
	enum class RequestType
	{
		NoRequest,

		// state requests
		//
		GetAppSignalState,
		GetAppSignalStateConstSize,

		// state changes requests
		//
		GetAppSignalStateChanges,
		GetGatewayAppSignalStateChanges
	};

public:
	GrpcAdsClient(const SoftwareInfo& localSoftwareInfo,
				  const std::vector<HostAddressPort>& serverAddress,
				  const QString& clientDescription,
				  CircularLoggerShared log,
				  const RequestType stateRequest,
				  size_t stateRequestInterval,
				  const RequestType stateChangesRequest,
				  size_t stateChangesMaxCount,
				  IAppSignalStateUpdaterShared updaterShared);

	virtual ~GrpcAdsClient();

	void setHashesToRequestStates(const std::vector<Hash>& hashes);
	void setHashesToRequestGatewayStateChanges(const std::vector<Hash>& hashes);

private:
	virtual void run() override;

	virtual void adsConnected() override;
	virtual void adsDisconnected() override;

	bool sendStateRequests();
	bool sendStateChangesRequests();

	bool sendGetAppSignalStateRequest(const Grpc::GetAppSignalStateRequest& request, bool constSizeRequest);
	bool sendGetAppStateChangesRequest(bool* hasPendingChanges);
	bool sendGetGatewayAppStateChangesRequest(bool* hasPendingChanges);

	void updateLastRequestTime(int64_t lastRequestTime = currentMSecsUTC());

	void fillGetStateRequest(Grpc::GetAppSignalStateRequest* request, bool* isLastPart);
	void fillGetGatewayStateChangesRequest(Grpc::GetGatewayAppSignalStateChangesRequest* request);

private:
	RequestType m_stateRequest = RequestType::GetAppSignalState;
	int64_t m_stateRequestInterval = 500;
	RequestType m_stateChangesRequest = RequestType::GetAppSignalStateChanges;
	int64_t m_stateChangesMaxCount = 5;

	IAppSignalStateUpdaterShared m_updaterShared;

	//

	std::mutex m_hashesToRequestStatesMutex;
	std::vector<Hash> m_hashesToRequestStates;
	size_t m_requestStateHashesIndex = 0;

	//

	std::mutex m_hashesToRequestGatewayStateChangesMutex;
	std::vector<Hash> m_hashesToRequestGatewayStateChanges;
	bool m_updateHashesToRequestGatewayStateChanges = true;

	//

	int64_t m_requestsCycleStartTime = 0;
	int64_t m_lastRequestTime = 0;
};

