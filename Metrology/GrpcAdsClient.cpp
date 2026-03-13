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
	IAppSignalStateUpdaterShared updater) :
	GrpcClient(localSoftwareInfo, serverAddress, clientDescription, log, false),
	m_updater(updater)
{
}

GrpcAdsClient::~GrpcAdsClient()
{
}

void GrpcAdsClient::setHashesToRequestStates(const std::vector<Hash>& hashes)
{
	std::lock_guard lg(m_hashesToRequestStatesMutex);
	m_hashesToRequestStates = hashes;
	m_requestStateHashesStartIndex = 0;
}

void GrpcAdsClient::setRequestTypes(const std::vector<RequestType>& requestTypes)
{
	if (requestTypes.size() == 0)
	{
		Q_ASSERT(false);
		requestTypes.push_back(RequestType::GetAppSignalState);
	}

	std::set uniqueRequestTypes;
	uniqueRequestTypes.insert(requestTypes.begin(), requestTypes.end());

	{
		std::lock_guard lg(m_requestTypesMutex);
		m_requestTypes.assign(uniqueRequestTypes.begin(), uniqueRequestTypes.end());
		m_requestTypeIndex = 0;
	}
}

void GrpcAdsClient::setStateRequestInterval(qint64 intervalMs)
{
   m_stateRequestInterval.store(std::clamp(intervalMs, 20, 3000));
}

void GrpcAdsClient::run()
{
	Grpc::GetAppSignalStateRequest stateRequest;
	bool isLastPart = false;

	RequestType requestType = getNextRequestType();

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

			updateLastRequestTime();
		}

		switch(requestType)
		{
		case RequestType::GetAppSignalState:
		case RequestType::GetappSignalStateConstSize:

			fillGetStateRequest(&stateRequest, &isLastPart);

			if (stateRequest.signalhashes_size() > 0)
			{
				if (sendGetAppSignalStateRequest(stateRequest) == true)
				{
					m_lastRequestTime = currentMSecsUTC();
				}
				else
				{
					m_lastRequestTime = 0;
					resetStub();
					continue;
				}
			}

			break;

		case RequestType::GetAppSignalStateChanges:
		case RequestType::GatewayGetAppSignalStateChanges:

		}

		//



		if (stateRequest.signalhashes_size() == 0 || isLastPart)
		{
			waitForOrQuit(50);
		}

		if (currentMSecsUTC() - m_lastRequestTime > pingPeriod())
		{
			if (sendPingRequest() == true)
			{
				m_lastRequestTime = currentMSecsUTC();

			}
			else
			{
				m_lastRequestTime = 0;
				resetStub();
			}
		}
	}

	resetStub();
}

GrpcAdsClient::RequestType GrpcAdsClient::getNextRequestType(bool& typesRestarted)
{
	std::lock_guard lg (m_requestTypesMutex);

	if (m_requestTypes.size() == 0)
	{
		Q_ASSERT(false);
		typesRestarted = true;
		return RequestType::GetAppSignalState;
	}

	if (m_requestTypeIndex > m_requestTypes.size())
	{
		m_requestTypeIndex = 0;
	}

	RequestType rt = m_requestTypes[m_requestTypeIndex];

	m_requestTypeIndex++;

	if (m_requestTypeIndex > m_requestTypes.size())
	{
		m_requestTypeIndex = 0;
		typesRestarted = true;
	}

	return rt;
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

		if (m_requestStateHashesStartIndex >= hashesCount)
		{
			m_requestStateHashesStartIndex = 0;
		}

		int count = 0;

		for(; m_requestStateHashesStartIndex < hashesCount && count < ADS_GET_APP_SIGNAL_STATE_MAX; m_requestStateHashesStartIndex++, count++)
		{
			Hash h = m_hashesToRequestStates[m_requestStateHashesStartIndex];
			request->add_signalhashes(h);
		}

		if (m_requestStateHashesStartIndex >= hashesCount)
		{
			m_requestStateHashesStartIndex = 0;
			*isLastPart = true;
		}
	}
}

bool GrpcAdsClient::sendGetAppSignalStateRequest(const Grpc::GetAppSignalStateRequest& request)
{
	grpc::ClientContext ctx;

	if (createContext(&ctx) == false)
	{
		return false;
	}

	Grpc::GetAppSignalStateReply reply;

	grpc::Status st = stub()->GetAppSignalState(&ctx, request, &reply);

	if (st.ok() == false)
	{
		return false;
	}

	if (m_updater != nullptr)
	{
		m_updater->updateAppSignalStates(reply);
	}

	return true;
}

