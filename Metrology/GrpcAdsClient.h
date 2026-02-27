#pragma once

#include "../OnlineLib/GrpcClient.h"


class GrpcAdsClient : public GrpcClient<Grpc::AppDataSrv>
{
	Q_OBJECT

public:
	GrpcAdsClient(const SoftwareInfo& localSoftwareInfo,
				  const std::vector<HostAddressPort>& serverAddress,
				  const QString& clientDescription,
				  CircularLoggerShared log);
	virtual ~GrpcAdsClient();

private:
	virtual void run() override;
	virtual void wakeupThread() override;
	virtual void createStubAndHandshake(grpc::Status* status = nullptr) override;
};

