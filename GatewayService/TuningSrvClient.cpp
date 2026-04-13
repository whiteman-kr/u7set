#include "TuningSrvClient.h"
#include "TuningGatewayServer.h"

TuningSrvClient::TuningSrvClient(std::shared_ptr<TgsSession> session,
	const SoftwareInfo& softwareInfo,
	const HostAddressPort& serverAddressPort1,
	const HostAddressPort& serverAddressPort2,
	const QString& clientDescription,
	const QString& serverEquipmentID) :
	Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2,
				clientDescription, serverEquipmentID)
{
}

void TuningSrvClient::onConnection()
{
}

void TuningSrvClient::onDisconnection()
{
}

void TuningSrvClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
}

TuningSrvClientThread::TuningSrvClientThread(std::shared_ptr<TgsSession> session,
											const SoftwareInfo& softwareInfo,
											const HostAddressPort& serverAddressPort1,
											const HostAddressPort& serverAddressPort2,
											const QString& clientDescription,
											const QString& serverEquipmentID)
{
	TuningSrvClient* client = new TuningSrvClient(session, softwareInfo,
												  serverAddressPort1, serverAddressPort2,
												  clientDescription, serverEquipmentID);
	addWorker(client);
}
