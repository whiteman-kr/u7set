#include <vector>
#include <set>
#include <algorithm>
#include <mutex>
#include <atomic>

#include "GrpcAdsClient.h"

#include <CommonStdLib/TimesStd.h>

GrpcAdsClient::GrpcAdsClient(const SoftwareInfo& localSoftwareInfo,
	const std::vector<HostAddressPort>& serverAddress,
	const QString& clientDescription,
	CircularLoggerShared log,
	const RequestType stateRequest,
	size_t stateRequestInterval,
	const RequestType stateChangesRequest,
	size_t stateChangesMaxCount,
	IAppSignalStateUpdaterShared updaterShared) :
	GrpcClient(localSoftwareInfo, serverAddress, clientDescription, log, 5000),
	m_updaterShared(updaterShared)
{
	Q_ASSERT(stateRequest == RequestType::NoRequest ||
			 stateRequest == RequestType::GetAppSignalState ||
			 stateRequest == RequestType::GetAppSignalStateConstSize);

	Q_ASSERT(stateChangesRequest == RequestType::NoRequest ||
			 stateChangesRequest == RequestType::GetAppSignalStateChanges ||
			 stateChangesRequest == RequestType::GetGatewayAppSignalStateChanges);

	Q_ASSERT(m_updaterShared != nullptr);

	static constexpr size_t MIN_STATE_REQUEST_INTERVAL = 10;
	static constexpr size_t MAX_STATE_REQUEST_INTERVAL = 2000;

	m_stateRequest = stateRequest;
	m_stateRequestInterval = std::clamp(stateRequestInterval,
										MIN_STATE_REQUEST_INTERVAL,
										MAX_STATE_REQUEST_INTERVAL);

	static constexpr size_t MIN_STATE_CHANGES_REQUEST_COUNT = 1;
	static constexpr size_t MAX_STATE_CHANGES_REQUEST_COUNT = 20;

	m_stateChangesRequest = stateChangesRequest;
	m_stateChangesMaxCount = std::clamp(stateChangesMaxCount,
										MIN_STATE_CHANGES_REQUEST_COUNT,
										MAX_STATE_CHANGES_REQUEST_COUNT);
}

GrpcAdsClient::~GrpcAdsClient()
{
}

void GrpcAdsClient::setHashesToRequestStates(const std::vector<Hash>& hashes)
{
	std::lock_guard lg(m_hashesToRequestStatesMutex);
	m_hashesToRequestStates = hashes;
	m_requestStateHashesIndex = 0;
}

void GrpcAdsClient::setHashesToRequestGatewayStateChanges(const std::vector<Hash>& hashes)
{
	std::lock_guard lg(m_hashesToRequestGatewayStateChangesMutex);
	m_hashesToRequestGatewayStateChanges = hashes;
	m_updateHashesToRequestGatewayStateChanges = true;
}

void GrpcAdsClient::run()
{
	while(isQuitRequested() == false)
	{
		if (authToken().empty())
		{
			createStubAndHandshake();

			if (authToken().empty())
			{
				waitForOrQuit(500);
				continue;
			}

			m_updateHashesToRequestGatewayStateChanges = true;
			m_requestsCycleStartTime = currentMSecsUTC();
			updateLastRequestTime();
		}

		m_requestsCycleStartTime = currentMSecsUTC();

		if (sendStateRequests() == false)
		{
			continue;
		}

		if (sendStateChangesRequests() == false)
		{
			continue;
		}

		int64_t pingRequestTime = m_lastRequestTime + pingPeriod();

		if (currentMSecsUTC() > pingRequestTime)
		{
			if (sendPingRequest() == true)
			{
				updateLastRequestTime();
			}
			else
			{
				m_lastRequestTime = 0;
				resetStub();
			}

			continue;
		}

		int64_t stateRequestTime = m_requestsCycleStartTime + m_stateRequestInterval;

		waitUntilOrQuit(stateRequestTime);
	}

	resetStub();
}

void GrpcAdsClient::adsConnected()
{
	if (m_updaterShared)
	{
		m_updaterShared->adsConnected();
	}
}

void GrpcAdsClient::adsDisconnected()
{
	if (m_updaterShared)
	{
		m_updaterShared->adsDisconnected();
	}
}

bool GrpcAdsClient::sendStateRequests()
{
	if (m_stateRequest == RequestType::NoRequest)
	{
		return true;
	}

	thread_local Grpc::GetAppSignalStateRequest request;

	bool isLastPart = false;

	while(isLastPart == false)
	{
		switch(m_stateRequest)
		{
		case RequestType::GetAppSignalState:
		case RequestType::GetAppSignalStateConstSize:

			request.Clear();
			fillGetStateRequest(&request, &isLastPart);

			if (request.signalhashes_size() > 0)
			{
				if (sendGetAppSignalStateRequest(request,
						m_stateRequest == RequestType::GetAppSignalStateConstSize) == true)
				{
					updateLastRequestTime();
				}
				else
				{
					resetStub();
					return false;
				}
			}
			else
			{
				isLastPart = true;
			}

			break;

		default:
			Q_ASSERT(false);
		}
	}

	return true;
}

bool GrpcAdsClient::sendStateChangesRequests()
{
	if (m_stateChangesRequest == RequestType::NoRequest)
	{
		return true;
	}

	int64_t requestCount = 0;
	bool hasPendingChanges = true;
	bool res = true;

	while(requestCount < m_stateChangesMaxCount &&
		   hasPendingChanges == true)
	{
		switch(m_stateChangesRequest)
		{
		case RequestType::GetAppSignalStateChanges:
			res = sendGetAppStateChangesRequest(&hasPendingChanges);
			requestCount++;
			break;

		case RequestType::GetGatewayAppSignalStateChanges:
			res = sendGetGatewayAppStateChangesRequest(&hasPendingChanges);
			break;

		default:
			Q_ASSERT(false);
		}

		if (res == false)
		{
			resetStub();
			return false;
		}

		updateLastRequestTime();
	}

	return true;
}

bool GrpcAdsClient::sendGetAppSignalStateRequest(const Grpc::GetAppSignalStateRequest& request,
												bool constSizeRequest)
{
	grpc::ClientContext ctx;

	if (createContext(&ctx) == false)
	{
		return false;
	}

	Grpc::GetAppSignalStateReply reply;

	grpc::Status st;

	if (constSizeRequest)
	{
		st = stub()->GetAppSignalStateConstSize(&ctx, request, &reply);

//		qDebug() << "Send GetAppSignalStateConstSize";
	}
	else
	{
		st = stub()->GetAppSignalState(&ctx, request, &reply);

//		qDebug() << "Send GetAppSignalState";
	}

	if (st.ok() == false)
	{
		return false;
	}

	if (m_updaterShared != nullptr)
	{
		m_updaterShared->updateAppSignalStates(reply);
	}

	return true;
}

bool GrpcAdsClient::sendGetAppStateChangesRequest(bool* hasPendingChanges)
{
	TEST_PTR_RETURN_FALSE(hasPendingChanges);

	*hasPendingChanges = false;

	grpc::ClientContext ctx;

	if (createContext(&ctx) == false)
	{
		return false;
	}

	thread_local Grpc::GetAppSignalStateChangesRequest request;
	thread_local Grpc::GetAppSignalStateChangesReply reply;

	reply.Clear();

	grpc::Status st = stub()->GetAppSignalStateChangesNoStream(&ctx, request, &reply);

	qDebug() << "Send GetAppSignalStateChangesNoStream";

	if (st.ok() == false)
	{
		return false;
	}

	if (m_updaterShared != nullptr)
	{
		m_updaterShared->processAppSignalStateChanges(reply);
	}

	*hasPendingChanges = (reply.pendingstatescount() > 0);

	return true;
}

bool GrpcAdsClient::sendGetGatewayAppStateChangesRequest(bool* hasPendingChanges)
{
	TEST_PTR_RETURN_FALSE(hasPendingChanges);

	*hasPendingChanges = false;

	grpc::ClientContext ctx;

	if (createContext(&ctx) == false)
	{
		return false;
	}

	thread_local Grpc::GetGatewayAppSignalStateChangesRequest request;
	thread_local Grpc::GetGatewayAppSignalStateChangesReply reply;

	request.Clear();
	reply.Clear();

	if (m_updateHashesToRequestGatewayStateChanges)
	{
		fillGetGatewayStateChangesRequest(&request);
		m_updateHashesToRequestGatewayStateChanges = false;
	}

	grpc::Status st = stub()->GetGatewayAppSignalStateChanges(&ctx, request, &reply);

	qDebug() << "Send GetGatewayAppSignalStateChanges";

	if (st.ok() == false)
	{
		return false;
	}

	if (m_updaterShared != nullptr)
	{
		m_updaterShared->processGatewayAppSignalStateChanges(reply);
	}

	*hasPendingChanges = (reply.pendingstatescount() > 0);

	return true;
}

void GrpcAdsClient::updateLastRequestTime(int64_t lastRequestTime)
{
	m_lastRequestTime = lastRequestTime;
}

void GrpcAdsClient::fillGetStateRequest(Grpc::GetAppSignalStateRequest* request, bool* isLastPart)
{
	TEST_PTR_RETURN(request);
	TEST_PTR_RETURN(isLastPart);

	*isLastPart = false;

	request->clear_signalhashes();

	{
		std::lock_guard lg(m_hashesToRequestStatesMutex);

		size_t hashesCount = m_hashesToRequestStates.size();

		if (hashesCount == 0)
		{
			*isLastPart = true;
			return;
		}

		if (m_requestStateHashesIndex >= hashesCount)
		{
			m_requestStateHashesIndex = 0;
		}

		for(int count = 0; m_requestStateHashesIndex < hashesCount &&
							count < ADS_GET_APP_SIGNAL_STATE_MAX;
							m_requestStateHashesIndex++, count++)
		{
			Hash h = m_hashesToRequestStates[m_requestStateHashesIndex];
			request->add_signalhashes(h);
		}

		if (m_requestStateHashesIndex >= hashesCount)
		{
			m_requestStateHashesIndex = 0;
			*isLastPart = true;
		}
	}
}

void GrpcAdsClient::fillGetGatewayStateChangesRequest(Grpc::GetGatewayAppSignalStateChangesRequest* request)
{
	TEST_PTR_RETURN(request);

	request->clear_signalhashes();

	{
		std::lock_guard lg(m_hashesToRequestGatewayStateChangesMutex);

		size_t hashesCount = m_hashesToRequestGatewayStateChanges.size();

		if (hashesCount == 0)
		{
			return;
		}

		size_t maxCount = std::min(hashesCount, TO_SIZE_T(ADS_GET_APP_SIGNAL_STATE_MAX));

		for(int count = 0; count < maxCount; count++)
		{
			Hash h = m_hashesToRequestGatewayStateChanges[count];
			request->add_signalhashes(h);
		}
	}
}

