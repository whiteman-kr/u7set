#include "GrpcSignalSocket.h"

GrpcSignalSocket::GrpcSignalSocket(const SoftwareInfo& swInfo,
									const QString& equipmentId1,
									const HostAddressPort& serverAddressPort1,
									const QString& equipmentId2,
									const HostAddressPort& serverAddressPort2,
									CircularLoggerShared log)
{
	Network::SoftwareInfo networkSoftwareInfo;
	swInfo.serializeTo(&networkSoftwareInfo);

	LoggerAdapter loggerAdapter(log);

	std::vector<ServiceEndpoint> services;

	ServiceEndpoint se;

	if (equipmentId1.isEmpty() == false)
	{
		se.equipmentId = equipmentId1.toStdString();
		se.shortenId = se.equipmentId;
		se.address.address = serverAddressPort1.addressStr().toStdString();
		se.address.port = serverAddressPort1.port();
		services.push_back(se);
	}

	if (equipmentId2.isEmpty() == false)
	{
		se.equipmentId = equipmentId2.toStdString();
		se.shortenId = se.equipmentId;
		se.address.address = serverAddressPort2.addressStr().toStdString();
		se.address.port = serverAddressPort2.port();
		services.push_back(se);
	}

	ClientLib::AdsConnection adsConnection(m_updater, nullptr, nullptr, loggerAdapter);
	adsConnection.updateConnections(networkSoftwareInfo, services);
}

GrpcSignalSocket::~GrpcSignalSocket()
{
}

bool GrpcSignalSocket::isConnected() const
{
	Q_ASSERT(false);	// TO DO
	return false;
}

int GrpcSignalSocket::selectedServerIndex() const
{
	Q_ASSERT(false);	// TO DO
	return 0;
}

void GrpcSignalSocket::configurationLoaded()
{
}




