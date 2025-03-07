#pragma once

#include <ServiceLib/Service.h>

#include "../OnlineLib/BuildInfo.h"
#include "../OnlineLib/UdpSocket.h"

struct ServiceData
{
	E::SoftwareType type = E::SoftwareType::Unknown;
	QString serviceName;

	quint16 port = 0;

	Network::ServiceInfo protoServiceInfo;

	OnlineLib::BuildInfo buildInfo;
	SoftwareInfo swInfo;
	SessionParams sessionParams;
	std::shared_ptr<SoftwareSettings> settings;

	std::vector<HostAddressPort> clientRequestIPs;

	UdpClientSocket* clientSocket = nullptr;
	QWidget* statusWidget = nullptr;

	//

	ServiceData();

	E::ServiceState serviceState() const;

	bool parseProtoServiceInfo();
	void fillClientRequestIPs(const std::vector<RqCtrlSettings>& rcSettings);
};

E::ServiceState serviceState(const Network::ServiceInfo& protoServiceInfo);
