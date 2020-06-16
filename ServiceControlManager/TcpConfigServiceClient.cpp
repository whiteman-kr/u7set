#include "TcpConfigServiceClient.h"
#include "../lib/SocketIO.h"


TcpConfigServiceClient::TcpConfigServiceClient(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& serverAddressPort) :
	Tcp::Client(softwareInfo, serverAddressPort, "TcpConfigServiceClient")
{
}


TcpConfigServiceClient::TcpConfigServiceClient(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& serverAddressPort1,
											   const HostAddressPort& serverAddressPort2) :
	Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2, "TcpConfigServiceClient")
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

	m_serviceStateIsReady = false;
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
		case CFGS_GET_SERVICE_STATE:
			onGetConfigurationServiceState(replyData, replyDataSize);
			break;

		case CFGS_GET_CLIENT_LIST:
			onGetConfigurationServiceClientList(replyData, replyDataSize);
			break;

		case CFGS_GET_LOADED_BUILD_INFO:
			onGetConfigurationServiceLoadedBuildInfoReply(replyData, replyDataSize);
			break;

		case CFGS_GET_SETTINGS:
			onGetConfigurationServiceSettingsReply(replyData, replyDataSize);
			break;

		default:
			assert(false);
	}
}

void TcpConfigServiceClient::onGetConfigurationServiceState(const char* replyData, quint32 replyDataSize)
{
	bool result = m_configurationServiceStateMessage.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	m_serviceStateIsReady = true;
	emit serviceStateLoaded();

	sendRequest(CFGS_GET_CLIENT_LIST);
}

void TcpConfigServiceClient::onGetConfigurationServiceClientList(const char* replyData, quint32 replyDataSize)
{
	bool result = m_serviceClientsMessage.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	m_clientsIsReady = true;
	emit clientsLoaded();

	sendRequest(CFGS_GET_LOADED_BUILD_INFO);
}


void TcpConfigServiceClient::onGetConfigurationServiceLoadedBuildInfoReply(const char* replyData, quint32 replyDataSize)
{
	Network::BuildInfo message;

	bool result = message.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	m_buildInfo.project = QString::fromStdString(message.project());

	m_buildInfo.id = message.id();

	m_buildInfo.date = QDateTime::fromMSecsSinceEpoch(message.date());

	m_buildInfo.changeset = message.changeset();

	m_buildInfo.user = QString::fromStdString(message.user());
	m_buildInfo.workstation = QString::fromStdString(message.workstation());

	m_buildInfoIsReady = true;
	emit buildInfoLoaded();

	sendRequest(CFGS_GET_SETTINGS);
}


void TcpConfigServiceClient::onGetConfigurationServiceSettingsReply(const char* replyData, quint32 replyDataSize)
{
	Network::ConfigurationServiceSettings message;

	bool result = message.ParseFromArray(replyData, replyDataSize);

	if (result == false)
	{
		assert(false);
		return;
	}

	m_equipmentID = QString::fromStdString(message.equipmentid());
	m_autoloadBuildPath = QString::fromStdString(message.autoloadbuildpath());
	m_workDirectory = QString::fromStdString(message.workdirectory());

	m_settingsIsReady = true;
	emit settingsLoaded();
}


void TcpConfigServiceClient::updateState()
{
	if (isClearToSendRequest())
	{
		sendRequest(CFGS_GET_SERVICE_STATE);
	}
}

