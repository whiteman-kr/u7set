#include "TcpConfigServiceClient.h"
#include "../lib/SocketIO.h"


TcpConfigServiceClient::TcpConfigServiceClient(const HostAddressPort& serverAddressPort) :
	Tcp::Client(serverAddressPort)
{
}


TcpConfigServiceClient::TcpConfigServiceClient(const HostAddressPort& serverAddressPort1, const HostAddressPort& serverAddressPort2) :
	Tcp::Client(serverAddressPort1, serverAddressPort2)
{
}


TcpConfigServiceClient::~TcpConfigServiceClient()
{
}


void TcpConfigServiceClient::onClientThreadStarted()
{

}


void TcpConfigServiceClient::onClientThreadFinished()
{

}


void TcpConfigServiceClient::onConnection()
{
	if (m_updateStatesTimer == nullptr)
	{
		m_updateStatesTimer = new QTimer(this);
		connect(m_updateStatesTimer, &QTimer::timeout, this, &TcpConfigServiceClient::updateState);
	}

	m_updateStatesTimer->start(200);

	updateState();
}


void TcpConfigServiceClient::onDisconnection()
{
	if (m_updateStatesTimer != nullptr)
	{
		m_updateStatesTimer->stop();
	}

	m_buildInfoIsReady = false;
	m_settingsIsReady = false;
	emit disconnected();
}


void TcpConfigServiceClient::onReplyTimeout()
{

}


void TcpConfigServiceClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	switch(requestID)
	{
		case RQID_GET_CONFIGURATION_SERVICE_LOADED_BUILD_INFO:
			onGetConfigurationSerivceLoadedBuildInfoReply(replyData, replyDataSize);
			break;

		case RQID_GET_CONFIGURATION_SERVICE_SETTINGS:
			onGetConfigurationSerivceSettingsReply(replyData, replyDataSize);
			break;

	default:
		assert(false);
	}
}


void TcpConfigServiceClient::onGetConfigurationSerivceLoadedBuildInfoReply(const char* replyData, quint32 replyDataSize)
{
	QByteArray data(replyData, replyDataSize);

	ConfigurationServiceBuildInfo info;

	info.readFromJson(data);

	m_buildInfo = info.buildInfo();

	m_buildInfoIsReady = true;
	emit buildInfoLoaded();

	sendRequest(RQID_GET_CONFIGURATION_SERVICE_SETTINGS);
}

void TcpConfigServiceClient::onGetConfigurationSerivceSettingsReply(const char* replyData, quint32 replyDataSize)
{
	QByteArray data(replyData, replyDataSize);

	ConfigurationServiceSettings s;

	s.readFromJson(data);

	m_settingsIsReady = true;
	emit settingsLoaded(s.equipmentID(), s.autoloadBuildPath(), s.workDirectory());
}

void TcpConfigServiceClient::updateState()
{
	sendRequest(RQID_GET_CONFIGURATION_SERVICE_LOADED_BUILD_INFO);
}

