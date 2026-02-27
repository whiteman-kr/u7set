#include "GrpcAdsClient.h"

GrpcAdsClient::GrpcAdsClient(const SoftwareInfo& localSoftwareInfo,
			  const std::vector<HostAddressPort>& serverAddress,
			  const QString& clientDescription,
			  CircularLoggerShared log) :
	GrpcClient(localSoftwareInfo, serverAddress, clientDescription, log, true)
{
}

GrpcAdsClient::~GrpcAdsClient()
{
}

void GrpcAdsClient::run()
{

}

void GrpcAdsClient::wakeupThread()
{

}

void GrpcAdsClient::createStubAndHandshake(grpc::Status* status )
{

}
