#include "GrpcAdsClient.h"

GrpcAdsClient::GrpcAdsClient(const SoftwareInfo& localSoftwareInfo,
	const std::vector<HostAddressPort>& serverAddress,
	const QString& clientDescription,
	CircularLoggerShared log,
	IAppSignalStateUpdaterShared updater) :
	GrpcClient(localSoftwareInfo, serverAddress, clientDescription, log, true),
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

void GrpcAdsClient::processing()
{
	Grpc::GetAppSignalStateRequest stateRequest;

	bool isLastPart = false;

	getStateRequest(&stateRequest, &isLastPart);

	if (stateRequest.signalhashes_size() > 0)
	{
		sendGetAppSignalStateRequest(stateRequest);
	}

	if (stateRequest.signalhashes_size() == 0 || isLastPart)
	{
		waitForOrQuit(50);
	}
}

void GrpcAdsClient::getStateRequest(Grpc::GetAppSignalStateRequest* request, bool* isLastPart)
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

void GrpcAdsClient::sendGetAppSignalStateRequest(const Grpc::GetAppSignalStateRequest& request)
{
	grpc::ClientContext ctx;

	if (createContext(&ctx) == false)
	{
		return;
	}

	Grpc::GetAppSignalStateReply reply;

	grpc::Status st = stub()->GetAppSignalState(&ctx, request, &reply);

	if (st.ok() == false)
	{
		resetStub();
		return;
	}

	if (m_updater != nullptr)
	{
		m_updater->updateAppSignalStates(reply);
	}
}

